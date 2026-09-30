function [u, v] = calculate_uv1976(spd, cmf)
    sn = normalize_irradiance(spd.val, cmf);

    X = sum(sn .* cmf.x * 5);
    Y = sum(sn .* cmf.y * 5);
    Z = sum(sn .* cmf.z * 5);

    u = 4 * X / (X + 15 * Y + 3 * Z);
    v = 9 * Y / (X + 15 * Y + 3 * Z);
end