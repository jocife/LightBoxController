function illuminant = read_illuminant(filename)

    t = readtable(filename, ...
        "DecimalSeparator", ",", ...
        "VariableNamingRule", "modify", ...
        "NumHeaderLines", 0);

    illuminant.wl = (300:5:780).';
    illuminant.D50.val = zeros(length(illuminant.wl), 1);
    illuminant.D55.val = zeros(length(illuminant.wl), 1);
    illuminant.D65.val = zeros(length(illuminant.wl), 1);
    illuminant.D75.val = zeros(length(illuminant.wl), 1);

    for i = 1:length(illuminant.wl)
        dex = find(t{:, 1} == illuminant.wl(i));
        if dex
            illuminant.D50.val(i) = t{dex, 5};
            illuminant.D55.val(i) = t{dex, 6};
            illuminant.D65.val(i) = t{dex, 3};
            illuminant.D75.val(i) = t{dex, 7};
        end
    end
end