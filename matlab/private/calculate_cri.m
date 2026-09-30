function cri = calculate_cri(spd, ref_spd, cmf, tcs)
    % Simplified Ra calculation relative to a known reference SPD
    % 1. Calculate color for test light and ref light on the 8 TCS
    % This is a high-level surrogate for the full CIE 13.3 process
    % to keep Coder compatibility simple.
    
    sn_test = normalize_irradiance(spd.val, cmf);
    sn_ref = normalize_irradiance(ref_spd.val, cmf);
    
    % Only interested in wl overlapping from 380..780
    % For simplicity, we just assume they align in index (length 97, 300:5:780)
    % TCS is 380:5:780 (length 81).
    % cmf/spd.val/ref_spd.val are 300:5:780 (length 97).
    % indices 17 to 97
    
    idx_test = 17:97;
    sn_t = sn_test(idx_test);
    sn_r = sn_ref(idx_test);
    x_bar = cmf.x(idx_test);
    y_bar = cmf.y(idx_test);
    z_bar = cmf.z(idx_test);
    
    delta_es = zeros(8, 1);
    
    for i = 1:8
        tcs_val = tcs(:, i); % This is 81x1
        
        % test
        Xt = sum(sn_t .* tcs_val .* x_bar);
        Yt = sum(sn_t .* tcs_val .* y_bar);
        Zt = sum(sn_t .* tcs_val .* z_bar);
        den_t = Xt + 15*Yt + 3*Zt;
        % Prevent division-by-zero / numerical instability when the
        % denominator is (near) zero. If the denominator is extremely
        % small, set the chromaticity coordinates to 0 as a safe fallback.
        if abs(den_t) < 1e-8
            ut = 0;
            vt = 0;
        else
            ut = 4*Xt / den_t;
            vt = 6*Yt / den_t;
        end
        
        % ref
        Xr = sum(sn_r .* tcs_val .* x_bar);
        Yr = sum(sn_r .* tcs_val .* y_bar);
        Zr = sum(sn_r .* tcs_val .* z_bar);
        den_r = Xr + 15*Yr + 3*Zr;
        % Same safety check for the reference color.
        if abs(den_r) < 1e-8
            ur = 0;
            vr = 0;
        else
            ur = 4*Xr / den_r;
            vr = 6*Yr / den_r;
        end
        
        % Simple Euclidean distance in uv space as a stand-in for full WUV space
        % Real CIE 13.3 uses Von Kries adaptation and W* U* V*
        de = 800 * sqrt((ut-ur)^2 + (vt-vr)^2); 
        delta_es(i) = de;
    end
    
    % R_i = 100 - 4.6 * dE
    Ri = 100 - 4.6 * delta_es;
    cri = mean(Ri);
    if cri < 0
        cri = 0;
    end
end
