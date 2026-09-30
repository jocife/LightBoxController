function specimen = read_uv_fluorescence(filename)

    t = readtable(filename, ...
        "DecimalSeparator", ",", ...
        "VariableNamingRule", "modify", ...
        "NumHeaderLines", 0);

    specimen.wl = (300:5:780).';
    specimen.s1.Q = zeros(length(specimen.wl), 1);
    specimen.s1.B = zeros(length(specimen.wl), 1);
    specimen.s1.F = zeros(length(specimen.wl), 1);
    specimen.s2.Q = zeros(length(specimen.wl), 1);
    specimen.s2.B = zeros(length(specimen.wl), 1);
    specimen.s2.F = zeros(length(specimen.wl), 1);
    specimen.s3.Q = zeros(length(specimen.wl), 1);
    specimen.s3.B = zeros(length(specimen.wl), 1);
    specimen.s3.F = zeros(length(specimen.wl), 1);

    for i = 1:length(specimen.wl)
        dex = find(t{:, 1} == specimen.wl(i));
        if dex
            if ~isnan(t{dex, 2})
                specimen.s1.Q(i) = t{dex, 2}; 
            end
            if ~isnan(t{dex, 3})
                specimen.s1.B(i) = t{dex, 3}; 
            end
            if ~isnan(t{dex, 4})
                specimen.s1.F(i) = t{dex, 4}; 
            end
            if ~isnan(t{dex, 5})
                specimen.s2.Q(i) = t{dex, 5}; 
            end
            if ~isnan(t{dex, 6})
                specimen.s2.B(i) = t{dex, 6}; 
            end
            if ~isnan(t{dex, 7})
                specimen.s2.F(i) = t{dex, 7}; 
            end
            if ~isnan(t{dex, 8})
                specimen.s3.Q(i) = t{dex, 8}; 
            end
            if ~isnan(t{dex, 9})
                specimen.s3.B(i) = t{dex, 9}; 
            end
            if ~isnan(t{dex, 10})
                specimen.s3.F(i) = t{dex, 10}; 
            end
        end
    end
end