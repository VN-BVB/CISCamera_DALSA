% 定义要处理的图像文件夹路径
imageFolder = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\qpg\';

% 获取文件夹下所有的 .bmp 文件
imageFileNames = dir(fullfile(imageFolder, '*.bmp'));
imageFileNames = fullfile(imageFolder, {imageFileNames.name});

% 检测图像中的棋盘格角点
[imagePoints, boardSize, imagesUsed] = detectCheckerboardPoints(imageFileNames);
imageFileNames = imageFileNames(imagesUsed);

% ==== 保存角点到txt文件 ====
saveDir = 'D:\Code\CISCamera_DALSA\data\CISCamera_Image\mattxt\';
if ~exist(saveDir, 'dir')
    mkdir(saveDir);
end

for i = 1:size(imagePoints, 3)
    pts = imagePoints(:, :, i);
    [~, name, ~] = fileparts(imageFileNames{i});
    timestamp = datestr(now, 'yyyymmdd_HHMMSS');
    txtFile = fullfile(saveDir, sprintf('Board%d_Points_%s.txt', i, timestamp));

    fid = fopen(txtFile, 'w');
    if fid ~= -1
        fprintf(fid, '# Index\tX\tY\n');
        for j = 0:size(pts, 1)
            fprintf(fid, '%d\t%.6f\t%.6f\n', j, pts(j,1), pts(j,2));
        end
        fclose(fid);
        fprintf('[保存完成] 棋盘格%d角点已写入：%s\n', i, txtFile);
    else
        fprintf('[错误] 无法创建输出文件：%s\n', txtFile);
    end
end
