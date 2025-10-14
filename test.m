tic;
I1 = mat2gray(imread("0.png"));
I2 = mat2gray(imread("1.png"));

I1 = imresize(I1, [512, 512]);
I2 = imresize(I2, [512, 512]);

[D, morphed] = diffeoDemon(I1, I2);
warped = imwarp(I2, D, 'interp', 'linear');


figure;
tiledlayout(1,3);
nexttile; imshow(I1, []); title('I1 (Reference)');
nexttile; imshow(morphed, []); title('MEX Warped');
nexttile; imshow(warped, []); title('imwarp Warped');