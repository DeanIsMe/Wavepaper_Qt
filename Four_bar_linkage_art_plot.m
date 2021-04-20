%% Four bar linkage art - simulation & plot
% Dean Reading 2021-01
% 2 arms ('a' and 'b'), each with 2 segments ('1' and '2')

%% Constants setup
xa_=0; % Base location of arm 'a'
ya_=0;
xb_=150;
yb_=0;
la1_=70; % Length of arm 'a' segment 'a'
lb1_=70;
la2_=300;
lb2_=300;

inca = 0.01; % Angle increment of arm 'a', segment '1'
incb = 0.0102;
ta1_ = 0; % Initial angle of arm 'a', segment '1'
tb1_ = 0;
stepCount = 20000; % Indicates when to stop drawing

visualizeProcess = 1; % Set to '1' if you want to visualize the plot as it's formed
speed_factor = 10; % Increase this to visualize faster. Integer.

    
%% Plot

% Equations:
% ta2 = 2*atan(((2*la2^2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 - (xa - xb + la1*cos(ta1) - lb1*cos(tb1))^4 - (ya - yb + la1*sin(ta1) - lb1*sin(tb1))^4 - 2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 + 2*lb2^2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 + 2*la2^2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2 + 2*lb2^2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2 - la2^4 - lb2^4 + 2*la2^2*lb2^2)^(1/2) - 2*la2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1)))/((xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 + (ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2 - 2*la2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1)) + la2^2 - lb2^2))
% Simplified to:
% ka = xa - xb + la1*cos(ta1) - lb1*cos(tb1)
% kb = ya - yb + la1*sin(ta1) - lb1*sin(tb1)
% kc = ka^2 + kb^2 - lb2^2
% ta2 = 2*atan((((2*la2*lb2)^2 - (kc - la2^2)^2)^(1/2) - 2*la2*kb)/(kc - 2*la2*ka + la2^2))

vec_x = zeros(stepCount, 1);
vec_y = zeros(stepCount, 1);
vec_ta2 = zeros(stepCount, 1);

tic

speed_factor = round(max(1, speed_factor));

for step = 1 : stepCount
    ka_ = xa_ - xb_ + la1_*cos(ta1_) - lb1_*cos(tb1_);
    kb_ = ya_ - yb_ + la1_*sin(ta1_) - lb1_*sin(tb1_);
    kc_ = ka_^2 + kb_^2 - lb2_^2;
    ta2_ = 2*atan((((2*la2_*lb2_)^2 - (kc_ - la2_^2)^2)^(1/2) - 2*la2_*kb_)/(kc_ - 2*la2_*ka_ + la2_^2));
    
    x3_ = xa_ + la1_*cos(ta1_) + la2_*cos(ta2_);
    y3_ = ya_ + la1_*sin(ta1_) + la2_*sin(ta2_);
    
    vec_x(step) = x3_;
    vec_y(step) = y3_; 
    vec_ta2(step) = ta2_;
    
    if visualizeProcess && mod(step, speed_factor) == 0
        drawx = [xa_, xa_ + la1_*cos(ta1_), xa_ + la1_*cos(ta1_) + la2_*cos(ta2_), xb_ + lb1_*cos(tb1_), xb_];
        drawy = [ya_, ya_ + la1_*sin(ta1_), ya_ + la1_*sin(ta1_) + la2_*sin(ta2_), yb_ + lb1_*sin(tb1_), yb_];

        clf

        plot(vec_x(1:step), vec_y(1:step), '-','LineWidth', 0.5);
        hold on
        plot(drawx, drawy, 'LineWidth', 3);
        
        title(sprintf('Step=%3d', step))
        xlim([-150, 350])
        ylim([-150, 400])
        drawnow
        pause(0.03)
    end
    
          
    % Increment angles
    ta1_ = ta1_ + inca;
    tb1_ = tb1_ + incb;
    %pause
end
toc
clf
plot(vec_x, vec_y)
axis square
