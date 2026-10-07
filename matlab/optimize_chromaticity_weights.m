function [weights] = optimize_chromaticity_weights(led_spd, led_wls, target_u, target_v) %#codegen
    params = coder.load('build_data/output/parameters.mat');
    cmf10 = params.cmf10;

    num_leds = size(led_spd, 2);
    x0 = ones(1, num_leds);
    options = optimoptions("fmincon", ...
        "Algorithm", "sqp", ...
        "ConstraintTolerance", 1e-4, ...
        "OptimalityTolerance", 1e-8, ...
        "Display", "none");

    lb = zeros(1, num_leds);
    ub = ones(1, num_leds);
    objective = @(weights) chromaticityLoss(weights, led_spd, led_wls, cmf10, target_u, target_v);

    [weights, ~, ~, ~] = fmincon(objective, x0, [], [], [], [], lb, ub, [], options);
end

function loss = chromaticityLoss(weights, led_spd, led_wls, cmf10, target_u, target_v)
    spd = prepare_spd(sum(abs(weights) .* led_spd, 2), led_wls);
    [u, v] = calculate_uv1976(spd, cmf10);
    loss = (u - target_u).^2 + (v - target_v).^2;
end
