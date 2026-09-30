function [weights] = optimize_led_weights(led_spd, led_wls, illuminant_type) %#codegen

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

    %% CIE STANDARD ILLUMINANT COORDINATES
    IL.D50.u = 0.2102;
    IL.D50.v = 0.4889;
    IL.D55.u = 0.2051;
    IL.D55.v = 0.4816;
    IL.D65.u = 0.1979;
    IL.D65.v = 0.4695;
    IL.D75.u = 0.1930;
    IL.D75.v = 0.4601;

    %% STANDARD UV SPECIMEN
    UV = params.UV;
    uvD50 = params.uvD50;
    uvD55 = params.uvD55;
    uvD65 = params.uvD65;
    uvD75 = params.uvD75;

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

    %% CALCULATE OPTIMIZATION
    fun = @(weights) simpleLoss( ...
        weights, led_spd, led_wls, cmf10, VIS, UV, ...
        target_vis, target_uv, target_il);
    
    % Optimization options
    % x0 = ones(1, length(led_names));
    num_leds = size(led_spd, 2);
    x0 = ones(1, num_leds);

    % The options must include the Algorithm option,
    % set to 'sqp' or 'sqp-legacy'.
    options = optimoptions("fmincon", ...
        "Algorithm", "sqp", ...             % "Algorithm", "interior-point", ...
        "ConstraintTolerance", 1e-4, ...
        "OptimalityTolerance", 1e-8, ...
        "Display", "none");                 % "Display", "iter-detailed"

    % lb = zeros(1, length(led_names));
    % ub = ones(1, length(led_names));
    
    lb = zeros(1, num_leds);
    ub = ones(1, num_leds);

    [weights, ~, ~, ~] = fmincon(fun, x0, [], [], [], [], ...
                                          lb, ub, [], options);
end

%% CORE FUNCTIONS
function [Duv, Mv, Mu] = calculateParameters( ...
                            weights, spectra, wavelength, ...
                            cmf, vis, uv, ...
                            target_vis, target_uv, target_il)

    spd = prepare_spd(sum(abs(weights).*spectra, 2), wavelength);

    Mv = calculate_mv(spd, cmf, target_vis, vis);
    Mu = calculate_mu(spd, cmf, target_uv, uv);

    [u, v] = calculate_uv1976(spd, cmf);
    Duv = sqrt((target_il.u - u).^2 + (target_il.v - v).^2);
end

%% LOSS FUNCTIONS
function loss = simpleLoss(varargin)
    [Duv, Mv, Mu] = calculateParameters(varargin{:});
    loss = Duv + Mv + Mu;
end


