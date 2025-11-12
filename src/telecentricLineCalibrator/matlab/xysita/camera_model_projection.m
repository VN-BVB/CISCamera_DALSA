function [pixel_u, pixel_v, in_image, u0, v0, img_width, img_height, inside] = ...
         camera_model_projection(X_world, Y_world, Z_world, inside)
%% ===================== 相机模型转换 =====================

% ------------------ 相机内参 ------------------
K = [47.277296561314515, -0.5909128336820076, 15551.864283815234;
     0.0, 46.88101813138735, 8051.201365000219;
     0.0, 0.0, 1.0];

coff_dis = [-5.44279886107428e-10, -6.607307708466992e-07, ...
            -3.85955640389808e-07, 2.082696492966601e-06, ...
            -2.0374950491798976e-06];

v_rot = [2.08338279543347, 2.0683810278122703, -0.20799695487856212];
v_trans = [-248.86297225612506, -135.65482873514011, 0.0];

img_width = 31104;
img_height = 16100;

u0 = 15551.864283815234;
v0 = 8049.999920255876;

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

R_2d = R_full(1:2,1:2);
R_ortho = eye(3);
R_ortho(1:2,1:2) = R_2d;
R_ortho(1:2,3)   = v_trans(1:2);

%% ========== 世界 -> 相机坐标 ==========
world_pts3 = [X_world'; Y_world'; ones(1, numel(X_world))];
camera_xy = R_ortho(1:2,:) * world_pts3;
camera_points = [camera_xy; ones(1, size(camera_xy,2))];

%% ========== 畸变与像素投影 ==========
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
pixel_u = pixel_coords(1,:);
pixel_v = pixel_coords(2,:);

% 判断哪些像素点落在图像范围
in_image = (pixel_u >= 0) & (pixel_u <= img_width) & ...
           (pixel_v >= 0) & (pixel_v <= img_height);

end
