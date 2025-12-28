function servo_on_cone_half_circle_small
% servo_on_cone_half_circle_small.m
% Mô phỏng servo trên hình nón – CON LĂN CHUYỂN ĐỘNG MƯỢT NHƯ THẬT
% Điều khiển bằng MPU6050 qua ESP32 (USB-TTL: COM5)

    close all; clc;

    %% ----- THAM SỐ MÔ HÌNH -----
    h = 1.2;           % Chiều cao đỉnh servo
    L = 1.0;           % Chiều dài tay servo
    alpha_deg = 35;
    alpha = deg2rad(alpha_deg);
    dt = 0.03;         % Thời gian bước (giây)

    %% ----- HÌNH HỌC NÓN -----
    R = L * sin(alpha);
    z_ray = h - L * cos(alpha);

    %% ----- VỊ TRÍ CỐ ĐỊNH -----
    theta_pos1 = pi/2;  % Bình thường (trên đỉnh)
    theta_pos2 = 0;    % Ngã trái
    theta_pos3 = pi;     % Ngã phải

    %% ----- KHỞI TẠO FIGURE -----
    fig = figure('Name','Servo on Half-Cone – Mượt Như Thật (MPU6050)','NumberTitle','off',...
        'Color',[1 1 1],'Units','normalized','Position',[0.1 0.1 0.8 0.8]);
    ax = axes('Parent',fig);
    hold(ax,'on'); grid(ax,'on'); axis(ax,'equal');
    view(ax,45,25);
    xlabel('X'); ylabel('Y'); zlabel('Z');
    title('Servo – Rod – Roller on Half-Cone (Chuyển động mượt)');
    lim = max([R L h]) + 0.3;
    xlim([-lim lim]); ylim([-lim lim]); zlim([0 h+0.3]);

    %% ----- VẼ HÌNH NÓN (NỀN) -----
    cone_base_R = R * 2.5;
    [XC, YC, ZC] = cylinder(linspace(0, cone_base_R, 50));
    ZC = h - ZC * h;
    surf(ax, XC, YC, ZC, 'FaceAlpha', 0.15, 'EdgeColor', 'none', 'FaceColor', [0.7 0.7 0.9]);

    %% ----- VẼ THANH RAY NỬA TRÒN -----
    rail_r = 0.04;
    [TH, PHI] = meshgrid(linspace(0, pi, 50), linspace(0, 2*pi, 16));
    rail_x = (R + rail_r * cos(PHI)) .* cos(TH);
    rail_y = (R + rail_r * cos(PHI)) .* sin(TH);
    rail_z = z_ray + rail_r * sin(PHI);
    surf(ax, rail_x, rail_y, rail_z, 'FaceColor', [0.8 0.8 0.8], 'EdgeColor', 'k', 'LineWidth', 0.2, 'FaceAlpha', 0.9);
    plot3(ax, R * cos(linspace(0, pi, 150)), R * sin(linspace(0, pi, 150)), z_ray * ones(1,150), 'k-', 'LineWidth', 1.5);

    %% ----- VẼ SERVO -----
    servo_w = 0.25; servo_d = 0.15; servo_h_body = 0.18; servo_z_base = h - servo_h_body;
    vertices = [ -servo_w/2, -servo_d/2, servo_z_base;
                  servo_w/2, -servo_d/2, servo_z_base;
                  servo_w/2,  servo_d/2, servo_z_base;
                 -servo_w/2,  servo_d/2, servo_z_base;
                 -servo_w/2, -servo_d/2, h;
                  servo_w/2, -servo_d/2, h;
                  servo_w/2,  servo_d/2, h;
                 -servo_w/2,  servo_d/2, h ];
    faces = [1 2 3 4; 5 6 7 8; 1 5 6 2; 2 6 7 3; 3 7 8 4; 4 8 5 1];
    patch(ax, 'Vertices', vertices, 'Faces', faces, 'FaceColor', [0.8 0.1 0.1], 'EdgeColor', 'k', 'LineWidth', 0.5, 'FaceAlpha', 0.9);

    %% ----- SERVO GEAR -----
    gear_r = 0.08; gear_h = 0.05;
    [SXg, SYg, SZg] = cylinder(linspace(0, gear_r, 10), 12);
    SZg = SZg * gear_h + (h - gear_h);
    surf(ax, SXg, SYg, SZg, 'FaceColor', [0.4 0.4 0.4], 'EdgeColor', 'k', 'LineWidth', 0.5);

    %% ----- ROD & ROLLER -----
    [Th_horn, Rh_horn] = meshgrid(linspace(0, 2*pi, 20), linspace(0, 0.12, 5));
    horn_x = Rh_horn .* cos(Th_horn);
    horn_y = Rh_horn .* sin(Th_horn);
    horn_z = h * ones(size(Th_horn));
    hornSurf = surf(ax, horn_x, horn_y, horn_z, 'FaceColor', [0.7 0.7 0.7], 'EdgeColor', 'k', 'FaceAlpha', 0.8);

    [sx, sy, sz] = sphere(24); roller_r = 0.1;
    rod = plot3(ax, [0 0], [0 R], [h z_ray], 'r-', 'LineWidth', 4);
    rollerSurf = surf(ax, roller_r*sx, R + roller_r*sy, z_ray + roller_r*sz, 'FaceColor', [1 0 0], 'EdgeColor', 'none');

    %% ----- ÁNH SÁNG -----
    camlight('headlight'); lighting gouraud;

    %% ----- STATUS TEXT -----
    statusTxt = uicontrol('Style', 'text', 'Units','normalized', ...
        'Position', [0.35 0.02 0.30 0.05], 'String', 'Status: Waiting...', ...
        'BackgroundColor', [1 1 1], 'FontWeight','bold', 'FontSize',11);

    %% ----- BIẾN CHUYỂN ĐỘNG MƯỢT -----
    current_theta = theta_pos1;
    target_theta  = theta_pos1;
    transition_steps = 40;        % Càng lớn càng mượt (40 ~ 1.2 giây)
    step_count = transition_steps; % Bắt đầu ở vị trí ổn định

    updatePose(current_theta);

    %% ----- HÀM CẬP NHẬT POSE -----
    function updatePose(theta)
        x = R * cos(theta);
        y = R * sin(theta);
        z = z_ray;
        set(rod, 'XData', [0 x], 'YData', [0 y], 'ZData', [h z]);
        set(rollerSurf, 'XData', x + roller_r*sx, 'YData', y + roller_r*sy, 'ZData', z + roller_r*sz);
        horn_x_new = Rh_horn .* cos(Th_horn + theta);
        horn_y_new = Rh_horn .* sin(Th_horn + theta);
        set(hornSurf, 'XData', horn_x_new, 'YData', horn_y_new, 'ZData', horn_z);
        drawnow limitrate;
    end

    %% ----- HÀM CHUYỂN ĐỘNG MƯỢT -----
    function moveSmoothlyTo(new_target)
        if abs(target_theta - new_target) > 0.01
            target_theta = new_target;
            step_count = 0;
        end
    end

    %% ----- HÀM TRẠNG THÁI -----
    function normalPose()
        moveSmoothlyTo(theta_pos1);
        set(statusTxt, 'String', 'Status: Bình Thường → Pos 1', 'ForegroundColor', 'green');
    end
    function fallRight()
        moveSmoothlyTo(theta_pos2);
        set(statusTxt, 'String', 'Status: Ngã Trái → Pos 2', 'ForegroundColor', 'red');
    end
    function fallLeft()
        moveSmoothlyTo(theta_pos3);
        set(statusTxt, 'String', 'Status: Ngã phải → Pos 3', 'ForegroundColor', 'blue');
    end
    function fallBack()
        moveSmoothlyTo(theta_pos1);
        set(statusTxt, 'String', 'Status: Ngã Ngửa → Về Pos 1', 'ForegroundColor', 'magenta');
    end

    %% ----- KẾT NỐI COM5 -----
    port = "COM5";
    try
        s = serialport(port, 115200);
        configureTerminator(s, "LF");
        flush(s);
        disp('Kết nối COM5 thành công! Đang nhận dữ liệu MPU6050...');
    catch
        error('Không kết nối được COM5! Kiểm tra USB-TTL đã cắm chưa?');
    end

    % Bộ lọc trung bình
    windowSize = 5;
    ax_buf = zeros(1,windowSize); ay_buf = ax_buf; az_buf = ax_buf;

    %% ----- VÒNG LẶP CHÍNH (CÓ CHUYỂN ĐỘNG MƯỢT) -----
    while ishandle(fig)
        % === CHUYỂN ĐỘNG MƯỢT ===
        if step_count < transition_steps
            step_count = step_count + 1;
            % Easing sin – chuyển động cực kỳ tự nhiên
            t = 0.5 - 0.5 * cos(pi * step_count / transition_steps);
            smooth_theta = current_theta + (target_theta - current_theta) * t;
            updatePose(smooth_theta);
            current_theta = smooth_theta;
        end

        % === ĐỌC DỮ LIỆU MPU6050 ===
        try
            if s.NumBytesAvailable > 0
                raw = readline(s);
                if startsWith(raw, "ACC")
                    data = split(raw, ",");
                    if length(data) >= 4
                        ax = str2double(data{2});
                        ay = str2double(data{3});
                        az = str2double(data{4});
                        if any(isnan([ax,ay,az])); continue; end

                        ax_buf = [ax_buf(2:end), ax];
                        ay_buf = [ay_buf(2:end), ay];
                        az_buf = [az_buf(2:end), az];
                        ax_f = mean(ax_buf); ay_f = mean(ay_buf); az_f = mean(az_buf);

                        % ĐIỀU KIỆN BÌNH THƯỜNG ĐÃ ĐƯỢC NỚI LỎNG
                        if az_f > 7.0 && abs(ax_f) < 5 && abs(ay_f) < 5
                            normalPose();
                        elseif ay_f < -7.0
                            fallRight();
                        elseif ay_f > 7.0
                            fallLeft();
                        elseif ax_f < -7.0
                            fallBack();
                        else
                            set(statusTxt, 'String', 'Status: Chưa xác định', 'ForegroundColor', [0.5 0.5 0.5]);
                        end

                        fprintf('AX=%.2f AY=%.2f AZ=%.2f → %s\n', ax_f, ay_f, az_f, get(statusTxt,'String'));
                    end
                end
            end
        catch
            pause(0.01);
        end
        pause(dt);
    end

    % Dọn dẹp
    delete(s); clear s;
end