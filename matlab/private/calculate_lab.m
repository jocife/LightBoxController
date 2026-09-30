function [L, a, b] = calculate_lab(X, Y, Z, Xn, Yn, Zn)

    rx = X/Xn;
    ry = Y/Yn;
    rz = Z/Zn;

    fx = 841/108 * rx + 16/116;
    fx(rx > (24/116)^3) = rx.^(1/3);

    fy = 841/108 * ry + 16/116;
    fy(ry > (24/116)^3) = ry.^(1/3);

    fz = 841/108 * rz + 16/116;
    fz(rz > (24/116)^3) = rz.^(1/3);

    L = 116 * fy - 16;
    a = 500*(fx-fy);
    b = 200*(fy-fz);

end