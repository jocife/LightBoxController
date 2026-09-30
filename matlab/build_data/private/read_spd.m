function [data, wavelengths, leds] = read_spd(filename, decimal)
    if nargin ~= 2
        decimal = ",";
    end

    t = readtable(filename, ...
        "DecimalSeparator", decimal, ...
        "VariableNamingRule", "preserve", ...
        "NumHeaderLines", 0);

    leds = t.Properties.VariableNames(2:end);
    wavelengths = t{:, 1};
    data = t{:, 2:end};

end