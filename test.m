tic;
I1 = mat2gray(imread("0.png"));
I2 = mat2gray(imread("1.png"));

I1 = imresize(I1, [512, 512]);
I2 = imresize(I2, [512, 512]);

[D, morphed] = diffeoDemon(I1, I2);
warped = imwarp(I2, D, 'interp', 'linear');

% figure("Name","Morphed"); imshow(morphed, []);
% figure("Name","Abs(D_tot)"); imshow(sqrt(D(:, :, 1) .^ 2 + D(:, :, 2) .^ 2), []);
% figure("Name","COMP - Source, Morphed"); imshowpair(I2, morphed);
% figure("Name","COMP - Morphed, imwarp"); imshowpair(morphed, imwarp(I2, D));
% figure("Name","Imwarp"); imshow(imwarp(I2, D), []);

% figure('Name', 'All Figures in One Window');
% tiledlayout(2, 2, 'TileSpacing', 'compact', 'Padding', 'compact');

% nexttile;
% imshow(morphed, []);
% title('Morphed');

% nexttile;
% imshow(sqrt(D(:, :, 1) .^ 2 + D(:, :, 2) .^ 2), []);
% title('Abs(D_{tot})');

% nexttile;
% imshowpair(morphed, imwarp(I2, D));
% title('COMP - Morphed, imwarp');

% nexttile;
% imshow(imwarp(I2, D), []);
% title('Imwarp');

% disp(toc);
% disp("breakpoint here");

figure;
tiledlayout(1,3);
nexttile; imshow(I1, []); title('I1 (Reference)');
nexttile; imshow(morphed, []); title('MEX Warped');
nexttile; imshow(warped, []); title('imwarp Warped');
