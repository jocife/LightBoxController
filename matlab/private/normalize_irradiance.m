function Sn = normalize_irradiance(S, cmf)
    Sn = 100 * S / sum(S .* cmf.y * 5);
    Sn(isnan(Sn)) = 0;
end