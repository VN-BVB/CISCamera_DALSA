function [K_best, R_best, t_best, final_MSE, Convergence_curve] = refine_linear_camera_BES(uv, ab, K0, R0, t0)
% 使用 BES 优化线阵相机的内参和多帧外参
% uv : cell array，每帧测量到的像素点 (2×N)
% ab : 标定板二维坐标 (2×N)
% K0 : 初始相机内参矩阵 (3x3)
% R0 : cell array，1xI，每帧初始旋转矩阵 (3x3)
% t0 : cell array，1xI，每帧初始平移向量 (3x1)

I = numel(uv);
N = size(ab,2);

%% 参数展开 -> 向量
params0 = pack_params(K0,R0,t0);
dim = numel(params0);

%% 搜索范围设置
low  = params0 - 0.5*abs(params0+1e-6); % 可调
high = params0 + 0.5*abs(params0+1e-6);

%% 目标函数
fobj = @(x) reprojection_error(x, uv, ab, I, N);

%% BES 参数
nPop = 100;
MaxIt = 2000;

[BestSol, Convergence_curve, ~] = BES(nPop, MaxIt, low, high, dim, fobj);

%% 解码结果
[K_best, R_best, t_best] = unpack_params(BestSol.pos, I);
final_MSE = BestSol.cost;

fprintf('BES优化后的重投影MSE: %f\n', final_MSE);
end

%% =================== 辅助函数 ===================

function params = pack_params(K,R,t)
    f  = K(1,1);
    u0 = K(1,3);
    s  = K(2,2);
    I  = numel(R);
    params = [f,u0,s];
    for i=1:I
        rvec = rotationMatrixToVector(R{i}); % 3参旋转向量
        params = [params, rvec(:)', t{i}(:)'];
    end
end

function [K,R,t] = unpack_params(params,I)
    f  = params(1); u0 = params(2); s = params(3);
    K = [f 0 u0; 0 s 0; 0 0 1];
    R = cell(1,I); t = cell(1,I);
    idx = 4;
    for i=1:I
        rvec = params(idx:idx+2); idx=idx+3;
        tvec = params(idx:idx+2); idx=idx+3;
        R{i} = rotationVectorToMatrix(rvec);
        t{i} = tvec(:);
    end
end

function err = reprojection_error(params, uv, ab, I, N)
    [K,R,t] = unpack_params(params,I);
%     fprintf('当前内参: f=%f, u0=%f, s=%f\n', K(1,1), K(1,3), K(2,2));

    total_err = 0;
    for i=1:I
        X = ab;  % planar points 2xN
        x_cam = R{i}*X + t{i};  % 3xN

        % 线阵相机投影方式
        uv_hat_x = K(1,1)*x_cam(1,:) ./ x_cam(3,:) + K(1,3);  % x方向归一化
        uv_hat_y = K(2,2)*x_cam(2,:);                           % y方向直接用，不除z

        uv_hat = [uv_hat_x; uv_hat_y];

        total_err = total_err + sum(sum((uv{i}(1:2,:) - uv_hat).^2));
    end

    err = total_err / (I*N);
end


%% =================== BES 主体算法 ===================
function [BestSol, Convergence_curve, timep] = BES(nPop, MaxIt, low, high, dim, fobj)
st = cputime;
% 初始化
BestSol.cost = inf;
pop.pos = zeros(nPop,dim);
pop.cost = zeros(nPop,1);
for i=1:nPop
    pop.pos(i,:) = low + (high-low).*rand(1,dim);
    pop.cost(i) = fobj(pop.pos(i,:));
    
    % 添加调试打印，显示每个个体的初始化位置和成本值
    fprintf('种群个体 %d 初始化成本: %f\n\n', i, pop.cost(i));
    
    if pop.cost(i) < BestSol.cost
        BestSol.pos  = pop.pos(i,:);
        BestSol.cost = pop.cost(i);
    end
end
% 打印初始最优解
fprintf('初始最优解位置:');
disp(BestSol.pos);
fprintf('初始最优解成本: %f\n', BestSol.cost);
disp(num2str([0 BestSol.cost]));

for t=1:MaxIt
    [pop, BestSol, ~] = select_space(fobj, pop, nPop, BestSol, low, high, dim);
    [pop, BestSol, ~] = search_space(fobj, pop, BestSol, nPop, low, high);
    [pop, BestSol, ~] = swoop(fobj, pop, BestSol, nPop, low, high);
    Convergence_curve(t) = BestSol.cost;
    % 记录并打印当前最优内参
    f_curr  = BestSol.pos(1);
    u0_curr = BestSol.pos(2);
    s_curr  = BestSol.pos(3);
    fprintf('Iter %03d | cost=%.6e | f=%.6f, u0=%.6f, s=%.6f\n', t, BestSol.cost, f_curr, u0_curr, s_curr);
end
timep = cputime - st;
end

function [pop, BestSol, s1] = select_space(fobj, pop, npop, BestSol, low, high, dim)
Mean = mean(pop.pos);
empty_individual.pos = [];
empty_individual.cost = [];
lm = 1.5;
s1 = 0;
for i=1:npop
    newsol = empty_individual;
    newsol.pos = BestSol.pos + lm*rand(1,dim).*(Mean - pop.pos(i,:));
    newsol.pos = max(min(newsol.pos, high), low);
    newsol.cost = fobj(newsol.pos);
    if newsol.cost < pop.cost(i)
       pop.pos(i,:) = newsol.pos;
       pop.cost(i) = newsol.cost;
       s1 = s1+1;
       if pop.cost(i) < BestSol.cost
           BestSol.pos = pop.pos(i,:);
           BestSol.cost = pop.cost(i);
       end
    end
end
end

function [pop, BestSol, s2] = search_space(fobj, pop, BestSol, npop, low, high)
Mean = mean(pop.pos);
a = 10; R = 1.5;
empty_individual.pos = [];
empty_individual.cost = [];
s2 = 0;
for i=1:npop-1
    A = randperm(npop); pop.pos = pop.pos(A,:); pop.cost = pop.cost(A);
    [x, y] = polr(a,R,npop);
    newsol = empty_individual;
    Step = pop.pos(i,:) - pop.pos(i+1,:);
    Step1= pop.pos(i,:) - Mean;
    newsol.pos = pop.pos(i,:) + y(i)*Step + x(i)*Step1;
    newsol.pos = max(min(newsol.pos, high), low);
    newsol.cost = fobj(newsol.pos);
    if newsol.cost < pop.cost(i)
       pop.pos(i,:) = newsol.pos;
       pop.cost(i) = newsol.cost;
       s2 = s2 + 1;
       if pop.cost(i) < BestSol.cost
           BestSol.pos = pop.pos(i,:);
           BestSol.cost = pop.cost(i);
       end
    end
end
end

function [pop, BestSol, s3] = swoop(fobj, pop, BestSol, npop, low, high)
Mean = mean(pop.pos);
a = 10; R = 1.5;
empty_individual.pos = [];
empty_individual.cost = [];
s3 = 0;
for i=1:npop
    A = randperm(npop); pop.pos = pop.pos(A,:); pop.cost = pop.cost(A);
    [x, y] = swoo_p(a,R,npop);
    newsol = empty_individual;
    Step = pop.pos(i,:) - 2*Mean;
    Step1= pop.pos(i,:) - 2*BestSol.pos;
    newsol.pos = rand(1,length(Mean)).*BestSol.pos + x(i)*Step + y(i)*Step1;
    newsol.pos = max(min(newsol.pos, high), low);
    newsol.cost = fobj(newsol.pos);
    if newsol.cost < pop.cost(i)
       pop.pos(i,:) = newsol.pos;
       pop.cost(i) = newsol.cost;
       s3 = s3 + 1;
       if pop.cost(i) < BestSol.cost
           BestSol.pos = pop.pos(i,:);
           BestSol.cost = pop.cost(i);
       end
    end
end
end

function [xR, yR] = swoo_p(a,R,N)
th = a*pi*exp(rand(N,1));
r = th;
xR = r.*sinh(th); yR = r.*cosh(th);
xR = xR/max(abs(xR)); yR = yR/max(abs(yR));
end

function [xR, yR] = polr(a,R,N)
th = a*pi*rand(N,1);
r  = th + R*rand(N,1);
xR = r.*sin(th); yR = r.*cos(th);
xR = xR/max(abs(xR)); yR = yR/max(abs(yR));
end
