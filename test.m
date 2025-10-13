I1 = mat2gray(imread("0.png"));
I2 = mat2gray(imread("1.png"));

I1 = imresize(I1, [512, 512]);
I2 = imresize(I2, [512, 512]);

[D, morphed] = diffeoDemon(I1, I2);

figure; imshow(morphed, []);
figure; imshow(sqrt(D(:, :, 1) .^ 2 + D(:, :, 2) .^ 2), []);
figure; imshowpair(I2, morphed);
figure; imshowpair(morphed, imwarp(I2, D));
figure; imshow(morphed, []);
disp("breakpoint here");
