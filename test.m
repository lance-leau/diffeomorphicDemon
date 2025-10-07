I1 = mat2gray(imread("0.png"));
I2 = mat2gray(imread("0.png"));

nIterations = 1;

[D, morphed] = diffeoDemon(I1, I2);

figure; imshow(morphed);
figure; imshow(sqrt(D(:, :, 1) .^ 2 + D(:, :, 2) .^ 2));
