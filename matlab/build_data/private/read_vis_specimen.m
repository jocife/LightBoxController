function specimen = read_vis_specimen(filename)

    t = readtable(filename, ...
        "DecimalSeparator", ",", ...
        "VariableNamingRule", "modify", ...
        "NumHeaderLines", 0);

    specimen.wl = (300:5:780).';
    specimen.s1 = zeros(length(specimen.wl), 1);
    specimen.s2 = zeros(length(specimen.wl), 1);
    specimen.s3 = zeros(length(specimen.wl), 1);
    specimen.s4 = zeros(length(specimen.wl), 1);
    specimen.s5 = zeros(length(specimen.wl), 1);

    wl = [t{:, 1}; t{:, 7}];
    s1 = [t{:, 2}; t{:, 8}];
    s2 = [t{:, 3}; t{:, 9}];
    s3 = [t{:, 4}; t{:, 10}];
    s4 = [t{:, 5}; t{:, 11}];
    s5 = [t{:, 6}; t{:, 12}];

    for i = 1:length(specimen.wl)
        dex = find(wl == specimen.wl(i));
        if dex
            specimen.s1(i) = s1(dex);
            specimen.s2(i) = s2(dex);
            specimen.s3(i) = s3(dex);
            specimen.s4(i) = s4(dex);
            specimen.s5(i) = s5(dex);
        end
    end
end