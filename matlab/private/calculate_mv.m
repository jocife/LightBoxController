function Mind = calculate_mv(spd, cmf, comparison, standard)
    Sn = normalize_irradiance(spd.val, cmf);

    svals = [standard.s1, standard.s2, standard.s3, ...
             standard.s4, standard.s5];

    cvals = [comparison.s1, comparison.s2, comparison.s3, ...
             comparison.s4, comparison.s5];

    Xn = sum(Sn .* cmf.x * 5);
    Yn = sum(Sn .* cmf.y * 5);
    Zn = sum(Sn .* cmf.z * 5);

    X1 = sum(svals .* Sn .* cmf.x * 5, 1);
    Y1 = sum(svals .* Sn .* cmf.y * 5, 1);
    Z1 = sum(svals .* Sn .* cmf.z * 5, 1);

    X2 = sum(cvals .* Sn .* cmf.x * 5, 1);
    Y2 = sum(cvals .* Sn .* cmf.y * 5, 1);
    Z2 = sum(cvals .* Sn .* cmf.z * 5, 1);

    [L1, a1, b1] = calculate_lab(X1, Y1, Z1, Xn, Yn, Zn);
    [L2, a2, b2] = calculate_lab(X2, Y2, Z2, Xn, Yn, Zn);

    dE = sqrt((L1 - L2).^2 + (a1 - a2).^2 + (b1 - b2).^2);
    Mind = mean(dE);
end