function [pixel_u, pixel_v, in_image, u0, v0, img_width, img_height, inside] = ...
         camera_model_projection(X_world, Y_world, Z_world, inside)
%% ===================== 相机模型转换 =====================
% 支持 vector 或 cell 输入

% ------------------ 相机内参 ------------------
K = [47.283237490301396, -0.657929607742621, 15551.964431991371;
     0.0,                 47.05230788272559, 8043.186819107249;
     0.0,                 0.0,               1.0];

coff_dis = [-5.363602460785097e-10, -6.586873205793823e-07, ...
            -3.9297031624526706e-07, 2.075287196873092e-06, ...
            -2.0074419972225162e-06];

% 新的旋转向量（v_rot）和平移向量（v_trans）
v_rot   = [2.074776703520814, 2.0584407379860554, -0.21906585124567024];
v_trans = [-249.01625128531074, -134.99191717289557, 0.0];

% 图像尺寸与主点（更新）
img_width  = 31104;
img_height = 16100;
u0 = 15551.964431991371;
v0 = 8049.979343505503;


%% ========== Rodrigues：旋转向量 -> 旋转矩阵 ==========
theta = norm(v_rot);
if theta < 1e-12
    R_full = eye(3);
else
    k_axis = v_rot(:) / theta;  
    Kskew = [   0       -k_axis(3)  k_axis(2);
             k_axis(3)    0       -k_axis(1);
            -k_axis(2)  k_axis(1)     0    ];
    R_full = eye(3) + sin(theta)*Kskew + (1 - cos(theta))*(Kskew*Kskew);
end

% 简化处理为 2D 投影仿真
R_2d = R_full(1:2,1:2);
R_ortho = eye(3);
R_ortho(1:2,1:2) = R_2d;
R_ortho(1:2,3)   = v_trans(1:2);

%% ========== 支持 cell 输入 ==========
if iscell(X_world)
    nCell = numel(X_world);
    pixel_u = cell(nCell,1);
    pixel_v = cell(nCell,1);
    in_image = cell(nCell,1);
    for i = 1:nCell
        [pixel_u{i}, pixel_v{i}, in_image{i}] = ...
            project_one(X_world{i}, Y_world{i}, Z_world{i}, inside);
    end
else
    [pixel_u, pixel_v, in_image] = project_one(X_world, Y_world, Z_world, inside);
end

%% ========== 内部函数：单批点集 ==========
    function [u_out, v_out, in_img] = project_one(X, Y, Z, inside_mask)
        world_pts3 = [X(:)'; Y(:)'; ones(1, numel(X))];
        camera_xy = R_ortho(1:2,:) * world_pts3;
        camera_points = [camera_xy; ones(1, size(camera_xy,2))];

        % --- 畸变与投影 ---
        normalized_x = camera_points(1,:);
        normalized_y = camera_points(2,:);
        [k1, h1, h2, s1, s2] = deal(coff_dis(1),coff_dis(2),coff_dis(3),coff_dis(4),coff_dis(5));
        r_sq = normalized_x.^2 + normalized_y.^2;
        deltaX = k1 .* normalized_x .* r_sq + ...
                 h1 .* (3*normalized_x.^2 + normalized_y.^2) + ...
                 2*h2 .* normalized_x .* normalized_y + ...
                 s1 .* r_sq;
        deltaY = k1 .* normalized_y .* r_sq + ...
                 2*h1 .* normalized_x .* normalized_y + ...
                 h2 .* (normalized_x.^2 + 3*normalized_y.^2) + ...
                 s2 .* r_sq;
        distorted_x = normalized_x + deltaX;
        distorted_y = normalized_y + deltaY;

        distorted_homo = [distorted_x; distorted_y; ones(1, numel(distorted_x))];
        pixel_coords = K * distorted_homo;
        u_out = pixel_coords(1,:);
        v_out = pixel_coords(2,:);

        % 判断是否落在图像内
        in_img = (u_out >= 0) & (u_out <= img_width) & ...
                 (v_out >= 0) & (v_out <= img_height);
        if nargin >= 4 && ~isempty(inside_mask)
            in_img = in_img & inside_mask(:)';
        end
    end

end
