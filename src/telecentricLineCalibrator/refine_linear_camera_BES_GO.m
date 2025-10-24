function [K_best, R_best, t_best, final_MSE, Convergence_curve] = refine_linear_camera_BES_GO(uv, ab, K0, R0, t0)
% refine_linear_camera_BESGO
% 使用 BES_GO 优化线阵相机的内参和多帧外参（完整程序）
% uv : cell array，每帧测量到的像素点 (2×N)
% ab : 标定板坐标 (3×N) 注意这里按你所述为 3xN
% K0 : 初始相机内参矩阵 (3x3)
% R0 : cell array，1xI，每帧初始旋转矩阵 (3x3)
% t0 : cell array，1xI，每帧初始平移向量 (3x1)

I = numel(uv);
N = size(ab,2);

%% 参数展开 -> 向量
params0 = pack_params(K0,R0,t0);
dim = numel(params0);

%% 搜索范围设置（默认 ±8%）
low  = params0 - 0.08*abs(params0+1e-6);
high = params0 + 0.08*abs(params0+1e-6);

% 防止焦距或尺度变成负值（可根据需要调整）
low(1) = max(low(1), 1);    % f >= 1
low(3) = max(low(3), 1e-6); % s >= tiny positive

%% 目标函数句柄（保留原参数顺序）
fobj = @(x) reprojection_error(x, uv, ab, I, N);

%% BES_GO 参数（你可以按需调整）
nPop = 180;
MaxIt = 800;

[fitness, best_pos, Convergence_curve] = BES_GO(nPop, MaxIt, low, high, dim, fobj);

%% 解码结果
[K_best, R_best, t_best] = unpack_params(best_pos, I);
final_MSE = fitness;

fprintf('BES_GO 优化后的重投影 MSE: %f\n', final_MSE);
end

%% ========== 辅助：pack / unpack ==========
function params = pack_params(K,R,t)
    f  = K(1,1);
    u0 = K(1,3);
    s  = K(2,2);
    I  = numel(R);
    params = [f,u0,s];
    for i=1:I
        rvec = rotm2rvec(R{i}); % 自实现 Rodrigues
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
        R{i} = rvec2rotm(rvec);
        t{i} = tvec(:);
    end
end

%% ========== reprojection error（保留原MSE除法） ==========
function err = reprojection_error(params, uv, ab, I, N)
    [K,R,t] = unpack_params(params,I);

    total_err = 0;
    for i=1:I
        X = ab;  % 3xN (你确认是3xN)
        x_cam = R{i}*X + repmat(t{i},1,size(X,2));  % 3xN

        % 线阵相机投影（按你原来实现）
        uv_hat_x = K(1,1)*x_cam(1,:) ./ x_cam(3,:) + K(1,3);  % x方向归一化
        uv_hat_y = K(2,2)*x_cam(2,:);                         % y方向直接用（线阵模型）

        uv_hat = [uv_hat_x; uv_hat_y];
        total_err = total_err + sum(sum((uv{i}(1:2,:) - uv_hat).^2));
    end

    err = total_err / (I*N); % 保持你原始的归一化方式
end

%% =================== Rodrigues 实现（不依赖 toolbox） ===================
function rvec = rotm2rvec(R)
    % 3x3 -> 3x1 Rodrigues 向量
    % 防止数值误差导致 acos 溢出
    tr = (trace(R)-1)/2;
    tr = min(max(tr, -1), 1);
    theta = acos(tr);
    if abs(theta) < 1e-12
        rvec = [0;0;0];
        return;
    end
    rx = (R(3,2)-R(2,3)) / (2*sin(theta));
    ry = (R(1,3)-R(3,1)) / (2*sin(theta));
    rz = (R(2,1)-R(1,2)) / (2*sin(theta));
    axis = [rx;ry;rz];
    rvec = axis * theta;
end

function R = rvec2rotm(rvec)
    theta = norm(rvec);
    if theta < 1e-12
        R = eye(3);
        return;
    end
    k = rvec / theta;
    K = [   0   -k(3)  k(2);
          k(3)    0   -k(1);
         -k(2)  k(1)    0   ];
    R = eye(3) + sin(theta)*K + (1-cos(theta))*(K*K);
end

%% =================== 你提供的 BES_GO 实现（已集成为局部函数） ===================
function [fitness, best_pos, Convergence_curve] = BES_GO(nPop,MaxIt,low,high,dim,fobj)
% nPop: size of population 
% MaxIt:number of iterations 
% low, high : space of Decision variables
% dim : number of Decision variables
% fobj : function handle

st=cputime;
disp('Running BES_GO')

% Initialize Best Solution
BestSol.cost = inf;

% pre-allocate population arrays
pop.pos = zeros(nPop, dim);
pop.cost = inf(nPop,1);

for i=1:nPop
    pop.pos(i,:) = low + (high-low).*rand(1,dim);
    pop.cost(i) = fobj(pop.pos(i,:));
    if pop.cost(i) < BestSol.cost
        BestSol.pos = pop.pos(i,:);
        BestSol.cost = pop.cost(i);
    end
end

for t=1:MaxIt
    % 1 - select_space
    [pop, BestSol, s1(t)] = select_space_GO(fobj, pop, nPop, BestSol, low, high, dim);

    % 2 - search_space (only first half iterations)
    if t <= round(MaxIt/2)
        [pop, BestSol, s2(t)] = search_space_GO(fobj, pop, BestSol, nPop, low, high);
    end

    % 3 - swoop (only first half iterations)
    if t <= round(MaxIt/2)
        [pop, BestSol, s3(t)] = swoop_GO(fobj, pop, BestSol, nPop, low, high);
    end

    % 4 - learning and Reflection Phase (only first half iterations)
    if t <= round(MaxIt/2)
        [pop, BestSol, s4(t)] = learning_phase_GO(fobj, pop, BestSol, nPop, low, high, MaxIt, t);
    end

    Convergence_curve(t) = BestSol.cost;
    ed = cputime;
    timep = ed - st; %#ok<NASGU>
    fitness = BestSol.cost;
    best_pos = BestSol.pos;
end
end

%% --------- BES_GO 子函数（独立命名避免冲突） ---------
function [pop, BestSol, s1] = select_space_GO(fobj, pop, npop, BestSol, low, high, dim)
Mean = mean(pop.pos);
empty_individual.pos = [];
empty_individual.cost = [];
lm = 2;
s1 = 0;
for i=1:npop
    newsol = empty_individual;
    newsol.pos = BestSol.pos + lm*rand(1,dim).*(Mean - pop.pos(i,:));
    newsol.pos = max(newsol.pos, low);
    newsol.pos = min(newsol.pos, high);
    newsol.cost = fobj(newsol.pos);
    if newsol.cost < pop.cost(i)
       pop.pos(i,:) = newsol.pos;
       pop.cost(i) = newsol.cost;
       s1 = s1 + 1;
       if pop.cost(i) < BestSol.cost
           BestSol.pos = pop.pos(i,:);
           BestSol.cost = pop.cost(i);
       end
    end
end
end

function [pop, best, s1] = search_space_GO(fobj, pop, best, npop, low, high)
Mean = mean(pop.pos);
a = 10;
R = 1.5;
empty_individual.pos = [];
empty_individual.cost = [];
s1 = 0;
for i=1:npop-1
    A = randperm(npop);
    pop.pos = pop.pos(A,:);
    pop.cost = pop.cost(A);
    [x, y] = polr_GO(a,R,npop);
    newsol = empty_individual;
    Step = pop.pos(i,:) - pop.pos(i+1,:);
    Step1 = pop.pos(i,:) - Mean;
    newsol.pos = pop.pos(i,:) + y(i)*Step + x(i)*Step1;
    newsol.pos = max(newsol.pos, low);
    newsol.pos = min(newsol.pos, high);
    newsol.cost = fobj(newsol.pos);
    if newsol.cost < pop.cost(i)
       pop.pos(i,:) = newsol.pos;
       pop.cost(i) = newsol.cost;
       s1 = s1 + 1;
       if pop.cost(i) < best.cost
           best.pos = pop.pos(i,:);
           best.cost = pop.cost(i);
       end
    end
end
end

function [pop, best, s1] = swoop_GO(fobj, pop, best, npop, low, high)
Mean = mean(pop.pos);
a = 10;
R = 1.5;
empty_individual.pos = [];
empty_individual.cost = [];
s1 = 0;
for i=1:npop
    A = randperm(npop);
    pop.pos = pop.pos(A,:);
    pop.cost = pop.cost(A);
    [x, y] = swoo_p_GO(a,R,npop);
    newsol = empty_individual;
    Step = pop.pos(i,:) - 2*Mean;
    Step1 = pop.pos(i,:) - 2*best.pos;
    newsol.pos = rand(1,length(Mean)).*best.pos + x(i)*Step + y(i)*Step1;
    newsol.pos = max(newsol.pos, low);
    newsol.pos = min(newsol.pos, high);
    newsol.cost = fobj(newsol.pos);
    if newsol.cost < pop.cost(i)
       pop.pos(i,:) = newsol.pos;
       pop.cost(i) = newsol.cost;
       s1 = s1 + 1;
       if pop.cost(i) < best.cost
           best.pos = pop.pos(i,:);
           best.cost = pop.cost(i);
       end
    end
end
end

function [pop, best, s1] = learning_phase_GO(fobj, pop, best, npop, low, high, MaxIt, t)
P1 = 5;
P2 = 0.001;
% P3 not used currently
[~, ind] = sort(pop.cost);
New_Best_Sol = pop.pos(ind(1), :);

s1 = 0;
for i = 1:npop
    Worst_Sol = pop.pos(ind(randi([npop - P1 + 1, npop], 1)), :);
    Better_Sol = pop.pos(ind(randi([2, P1], 1)), :);
    random = selectID_GO(npop, i, 2);
    L1 = random(1);
    L2 = random(2);

    % Gap calculations
    Gap1 = (New_Best_Sol - Better_Sol);
    Gap2 = (New_Best_Sol - Worst_Sol);

    % Distance calculations
    Distance1 = norm(Gap1);
    Distance2 = norm(Gap2);

    SumDistance = Distance1 + Distance2;
    if SumDistance == 0
        LF1 = 0.5; LF2 = 0.5;
    else
        LF1 = Distance1 / SumDistance;
        LF2 = Distance2 / SumDistance;
    end

    % Scaling factor
    SF = (pop.cost(i) / max(pop.cost));

    % New position calculation
    KA1 = LF1 * SF * Gap1;
    KA2 = LF2 * SF * Gap2;
    newsol_i = pop.pos(i, :) + KA1 + KA2;

    % Clipping to boundary
    newsol_i = max(newsol_i, low);
    newsol_i = min(newsol_i, high);
    newfitness = feval(fobj, newsol_i);

    % Update population
    if pop.cost(i) > newfitness
        pop.cost(i) = newfitness;
        pop.pos(i, :) = newsol_i;
        s1 = s1 + 1;
    elseif rand < P2 && ind(i) ~= 1
        pop.cost(i) = newfitness;
        pop.pos(i, :) = newsol_i;
    end

    % Update global best
    if best.cost > pop.cost(i)
         best.cost  = pop.cost(i);
         best.pos = pop.pos(i, :);
    end
end
end

function [xR, yR] = swoo_p_GO(a,R,N)
th = a*pi*exp(rand(N,1));
r  = th;
xR = r.*sinh(th);
yR = r.*cosh(th);
xR = xR / max(abs(xR));
yR = yR / max(abs(yR));
end

function [xR, yR] = polr_GO(a,R,N)
th = a*pi*rand(N,1);
r  = th + R*rand(N,1);
xR = r.*sin(th);
yR = r.*cos(th);
xR = xR / max(abs(xR));
yR = yR / max(abs(yR));
end

function [r] = selectID_GO(npop, i, k)
% Generate k random integers within [1, popsize] excluding i
if k <= npop
    vecc = [1:i-1, i+1:npop];
    r = zeros(1, k);
    for idx = 1:k
        n = numel(vecc);
        t = randi(n, 1);
        r(idx) = vecc(t);
        vecc(t) = [];
    end
else
    r = [];
end
end
