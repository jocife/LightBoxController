function [Mu, Mv, Duv, CRI, CCT, test_spd_out, ref_spd_out] = evaluate_metrics(weights, led_spd, led_wls, illuminant_type) %#codegen
    % evaluate_metrics calculates colorimetry metrics for a given set of LED weights
    % without running an optimization algorithm.

    %% Load all static standard reference data from parameters.mat
    params = coder.load('build_data/output/parameters.mat');

    %% COLOR MATCHING FUNCTION
    cmf10 = params.cmf10;

    %% STANDARD VIS SPECIMEN
    VIS = params.VIS;
    D50 = params.D50;
    D55 = params.D55;
    D65 = params.D65;
    D75 = params.D75;

    %% CIE STANDARD ILLUMINANTS
    IL = params.IL;

    %% STANDARD UV SPECIMEN
    UV = params.UV;
    uvD50 = params.uvD50;
    uvD55 = params.uvD55;
    uvD65 = params.uvD65;
    uvD75 = params.uvD75;

    %% TCS
    TCS = params.TCS;
    tcs_matrix = [TCS.TCS01, TCS.TCS02, TCS.TCS03, TCS.TCS04, TCS.TCS05, TCS.TCS06, TCS.TCS07, TCS.TCS08];

    %% CHOOSE ILLUMINANT TYPE
    switch illuminant_type
        case 0
            target_vis = D50;
            target_uv = uvD50;
            target_il = IL.D50;
        case 1
            target_vis = D55;
            target_uv = uvD55;
            target_il = IL.D55;
        case 2
            target_vis = D65;
            target_uv = uvD65;
            target_il = IL.D65;
        case 3
            target_vis = D75;
            target_uv = uvD75;
            target_il = IL.D75;
        otherwise
            warning('Unexpected value. Defaulting to D50.');
            target_vis = D50;
            target_uv = uvD50;
            target_il = IL.D50;
    end

    %% GENERATE SPECTRUM
    % weights should be a 1 x num_leds array, led_spd is 81 x num_leds
    % if weights is passed as a column vector, transpose it (led_spd is columns of LEDs)
    if size(weights, 2) == 1
        weights = weights';
    end
    
    spd = prepare_spd(sum(weights .* led_spd, 2), led_wls);
    
    %% CALCULATE METRICS
    Mv = calculate_mv(spd, cmf10, target_vis, VIS);
    Mu = calculate_mu(spd, cmf10, target_uv, UV);

    [u, v] = calculate_uv1976(spd, cmf10);
    Duv = sqrt((target_il.u - u).^2 + (target_il.v - v).^2);

    CCT = calculate_cct(spd, cmf10);
    CRI = calculate_cri(spd, target_il, cmf10, tcs_matrix);
    
    % Return the SPDs for plotting in the GUI (380-780nm)
    % Return raw (unnormalized) SPDs; the GUI may normalize for display.
    test_spd = spd.val(17:97);
    ref_spd = target_il.val(17:97);

    test_spd_out = test_spd;
    ref_spd_out = ref_spd;
end
