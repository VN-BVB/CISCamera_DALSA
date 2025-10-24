function [K_opt, R_opt, t_opt, theta_opt, k_opt, final_RMS] = refine_telecentric_camera(uv, ab, K_init, R_init, t_init, dx, dy)
% LM 优化远心相机参数（带逐帧重投影误差打印）
% uv: cell，每帧像素点 [1xN x2]
% ab: [2 x N] 标定板坐标 mm
% K_init: 初始内参
% R_init/t_init: 每帧初始外参
% dx, dy: mm/pixel
% 输出 K_opt, R_opt, t_opt, theta_opt, k_opt, final_RMS (像素单位)

    numImages = numel(R_init);
    N = size(ab,2);

    %% -------------------- 初始参数 --------------------
    m0 = K_init(1,1) * dx;   % mm->像素缩放
    u0 = K_init(1,3);
    v0 = K_init(2,3);
    theta0 = 0;
    k0 = 0;

    % 打包 p = [m; theta; k; rvec1; t1; rvec2; t2; ...]
    p = zeros(3 + numImages*6,1);
    p(1)=m0; p(2)=theta0; p(3)=k0;
    for i=1:numImages
        rvec = rotMatToVec(R_init{i});
        p(3+(i-1)*6 + (1:3)) = rvec(:);
        p(3+(i-1)*6 + (4:6)) = t_init{i}(:);
    end

    %% -------------------- 参数缩放 --------------------
    scaleVec = ones(size(p));
    scaleVec(1) = max(abs(p(1)),1);
    scaleVec(2) = max(abs(p(2)),1e-3);
    scaleVec(3) = max(abs(p(3)),1e-6);
    for i=1:numImages
        idx = 3 + (i-1)*6;
        scaleVec(idx+(1:3)) = max(abs(p(idx+(1:3))),1e-2);
        scaleVec(idx+(4:6)) = max(abs(p(idx+(4:6))),10);
    end
    p_scaled = p ./ scaleVec;

    %% -------------------- LM 参数 --------------------
    max_iter = 100;
    lambda = 1e-2;
    eps_err = 1e-9;
    eps_param = 1e-8;

    %% -------------------- 初始残差 --------------------
    r0 = residuals_pixel_cpp(p_scaled, scaleVec, uv, ab, dx, dy, u0, v0);
    r_u = r0(1:2:end);
    r_v = r0(2:2:end);
    r_u_mat = reshape(r_u, N, numImages);
    r_v_mat = reshape(r_v, N, numImages);
    frame_RMS = sqrt(mean(r_u_mat.^2 + r_v_mat.^2, 1)); % 1 x numImages
    prev_RMS = mean(frame_RMS);

    fprintf('初始 RMS（按单帧平均）: %.6f px\n', prev_RMS);

    %% -------------------- LM 迭代 --------------------
    for iter=1:max_iter
        J = numerical_jacobian_scaled(@(ps) residuals_pixel_cpp(ps, scaleVec, uv, ab, dx, dy, u0, v0), p_scaled);
        r = residuals_pixel_cpp(p_scaled, scaleVec, uv, ab, dx, dy, u0, v0);

        JTJ = J'*J; JTr = J'*r;
        diagJTJ = diag(JTJ); diagJTJ(diagJTJ==0)=1e-6;
        A = JTJ + lambda*diagJTJ;
        dp_scaled = -pinv(A)*JTr;

        % 尝试更新
        p_try = p_scaled + dp_scaled;
        r_try = residuals_pixel_cpp(p_try, scaleVec, uv, ab, dx, dy, u0, v0);

        r_u_try = r_try(1:2:end);
        r_v_try = r_try(2:2:end);
        r_u_mat_try = reshape(r_u_try, N, numImages);
        r_v_mat_try = reshape(r_v_try, N, numImages);
        frame_RMS_try = sqrt(mean(r_u_mat_try.^2 + r_v_mat_try.^2, 1));
        RMS_try = mean(frame_RMS_try);

        if RMS_try < prev_RMS
            p_scaled = p_try;
            prev_RMS = RMS_try;
            lambda = lambda*0.1;
            if prev_RMS<eps_err || norm(dp_scaled)<eps_param
                fprintf('LM 收敛: iter=%d, RMS=%.6f px\n', iter, prev_RMS);
                break;
            end
        else
            lambda = lambda*10;
        end
        fprintf('iter %2d: RMS=%.6f px, ||dp||=%.6e, lambda=%.3e\n', iter, prev_RMS, norm(dp_scaled), lambda);
    end

    %% -------------------- 解包参数 --------------------
    p_opt = p_scaled .* scaleVec;
    m_opt = p_opt(1); theta_opt = p_opt(2); k_opt = p_opt(3);
    R_opt = cell(numImages,1); t_opt = cell(numImages,1);
    for i=1:numImages
        base = 3 + (i-1)*6;
        rvec = p_opt(base+(1:3));
        t_opt{i} = p_opt(base+(4:6));
        R_opt{i} = rodrigues_rvec_to_mat(rvec);
    end
    K_opt = [m_opt/dx, -m_opt*tan(theta_opt)/dx, u0;
             0, 1/(dy*cos(theta_opt)), v0/m_opt;
             0, 0, 1];

    %% -------------------- 重投影误差逐帧计算 --------------------
    fprintf('\n--- 重投影误差变化 ---\n');
    reprojErr_opt = zeros(numImages,1);
    initIntrinsics.m = K_init(1,1) * dx;
    initIntrinsics.u0 = K_init(1,3);
    initIntrinsics.v0 = K_init(2,3);
    initIntrinsics.dx = dx;
    initIntrinsics.dy = dy;
    allImagePoints = uv;

    for i = 1:numImages
        uv_proj = zeros(2, N);
        R2 = R_opt{i}(1:2,1:2);
        t2 = t_opt{i}(1:2);
        img_affine = R2 * ab + repmat(t2,1,N);
        x_u = img_affine(1,:);
        y_u = img_affine(2,:);
        r_sq = x_u.^2 + (initIntrinsics.v0 * initIntrinsics.dy)^2;
        delta_x = k_opt * x_u .* r_sq;
        delta_y = -k_opt * initIntrinsics.v0 * initIntrinsics.dy .* r_sq;
        x_d = x_u + delta_x;
        y_d = y_u + delta_y;
        hat_u = (initIntrinsics.m / initIntrinsics.dx) * x_d ...
                - (initIntrinsics.m * tan(theta_opt) / initIntrinsics.dx) * y_d ...
                + initIntrinsics.u0;
        hat_v = (1 / (initIntrinsics.dy * cos(theta_opt))) * y_d ...
                + (initIntrinsics.v0 / initIntrinsics.m);
        uv_proj(1,:) = hat_u;
        uv_proj(2,:) = hat_v;

        actual = squeeze(allImagePoints{i}(1,:,1:2))';
        diff = uv_proj - actual;
        err_sq = sum(diff.^2,1);
        reprojErr_opt(i) = sqrt(mean(err_sq(:)));

        fprintf('图像 %02d: %.4f px\n', i, reprojErr_opt(i));
    end

    fprintf('平均重投影误差: %.4f px\n', mean(reprojErr_opt));
    final_RMS = mean(reprojErr_opt);
end

%% -------------------- 像素残差 --------------------
function residual_vec = residuals_pixel_cpp(p_scaled, scaleVec, uv, ab, dx, dy, u0, v0)
    p = p_scaled .* scaleVec;
    numImages = numel(uv); N = size(ab,2);
    residual_vec = zeros(2*numImages*N,1);
    m = p(1); theta = p(2); k = p(3);
    for i=1:numImages
        base = 3 + (i-1)*6;
        rvec = p(base+(1:3));
        tvec = p(base+(4:6));
        R = rodrigues_rvec_to_mat(rvec);
        R2 = R(1:2,1:2); t2 = tvec(1:2);
        xy_aff = R2*ab + t2;
        x_u = xy_aff(1,:); y_u = xy_aff(2,:);
        r_sq = x_u.^2 + (v0*dy)^2;
        x_d = x_u + k*x_u.*r_sq;
        y_d = y_u - k*v0*dy*r_sq;
        hat_u = (m/dx)*x_d - (m*tan(theta)/dx)*y_d + u0;
        hat_v = (1/(dy*cos(theta)))*y_d + v0/m;
        actual_u = squeeze(uv{i}(1,:,1));
        actual_v = squeeze(uv{i}(1,:,2));
        idx_u = 2*N*(i-1) + (1:N);
        idx_v = idx_u + N;
        residual_vec(idx_u) = actual_u(:) - hat_u(:);
        residual_vec(idx_v) = actual_v(:) - hat_v(:);
    end
end

%% -------------------- 数值雅可比 --------------------
function J = numerical_jacobian_scaled(fun,p)
    f0 = fun(p); n=numel(p); m=numel(f0); J=zeros(m,n); eps_base=1e-6;
    for k=1:n
        h = eps_base*max(1,abs(p(k)));
        dp=zeros(n,1); dp(k)=h;
        J(:,k) = (fun(p+dp)-fun(p-dp))/(2*h);
    end
end

%% -------------------- rvec <-> R --------------------
function rvec = rotMatToVec(R)
    tr = trace(R); cos_theta = (tr-1)/2; cos_theta = min(1,max(-1,cos_theta));
    theta = acos(cos_theta);
    if theta<1e-12, rvec=[0;0;0]; return; end
    rvec = theta/(2*sin(theta)) * [R(3,2)-R(2,3); R(1,3)-R(3,1); R(2,1)-R(1,2)];
end

function R = rodrigues_rvec_to_mat(rvec)
    theta = norm(rvec);
    if theta<1e-12, R=eye(3); return; end
    k = rvec/theta; K=[0,-k(3),k(2); k(3),0,-k(1); -k(2),k(1),0];
    R = eye(3)+sin(theta)*K+(1-cos(theta))*(K*K);
end
