function cmf = prepare_cmf(filepath)
    data = readtable(filepath);
    cmf.wl = (300:5:780).';

    cmf.x = zeros(length(cmf.wl), 1);
    cmf.y = zeros(length(cmf.wl), 1);
    cmf.z = zeros(length(cmf.wl), 1);

    for i = 1:length(cmf.wl)
        dex = find(data{:, 1} == cmf.wl(i));
        if dex
            cmf.x(i) = data{dex, 2};
            cmf.y(i) = data{dex, 3};
            cmf.z(i) = data{dex, 4};
        end
    end
end