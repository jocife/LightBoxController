function cct = calculate_cct(spd, cmf)
    % Calculate CCT using McCamy's formula
    
    sn = normalize_irradiance(spd.val, cmf);

    X = sum(sn .* cmf.x * 5);
    Y = sum(sn .* cmf.y * 5);
    Z = sum(sn .* cmf.z * 5);

    x = X / (X + Y + Z);
    y = Y / (X + Y + Z);
    
    n = (x - 0.3320) / (0.1858 - y);
    cct = -449 * n^3 + 3525 * n^2 - 6823.3 * n + 5520.33;
end
