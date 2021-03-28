%% Constants setup
xa_=0;
ya_=0;
xb_=150;
yb_=0;
la1_=70;
lb1_=70;
la2_=300;
lb2_=300;

%% Set up equations
syms ta2 tb2;

syms ta1 tb1;
syms xa ya xb yb;
syms la1 lb1 la2 lb2;
syms kx ky kz;

sub_var = {xa, ya, xb, yb, la1, lb1, la2, lb2};
sub_val = {xa_, ya_, xb_, yb_, la1_, lb1_, la2_, lb2_};

eqn1 = sym(kx == la2*cos(ta2) - lb2*cos(tb2));
eqn2 = sym(ky == la2*sin(ta2) - lb2*sin(tb2));
eqn3 = sym(kz == tb2 - ta2);

eqn_kx = sym(kx == xb - xa - la1*cos(ta1) + lb1*cos(tb1));
eqn_ky = sym(ky == yb - ya - la1*sin(ta1) + lb1*sin(tb1));
eqn_kz = sym(kz == acos((la2^2 + lb2^2 - (xb + lb1*cos(tb1) - xa - la1*cos(ta1))^2 - (yb + lb1*sin(tb1) - ya - la1*sin(ta1))^2)/(2*la2*lb2)));

expr_kx = sym(xb - xa - la1*cos(ta1) + lb1*cos(tb1));
expr_ky = sym(yb - ya - la1*sin(ta1) + lb1*sin(tb1));
expr_kz = sym(acos((la2^2 + lb2^2 - (xb + lb1*cos(tb1) - xa - la1*cos(ta1))^2 - (yb + lb1*sin(tb1) - ya - la1*sin(ta1))^2)/(2*la2*lb2)));

exprv_kx = subs(expr_kx, sub_var, sub_val);
exprv_ky = subs(expr_ky, sub_var, sub_val);
exprv_kz = subs(expr_kz, sub_var, sub_val);

eqn_x3 = sym(xa + la1*cos(ta1) + la2*cos(ta2));
eqn_y3 = sym(ya + la1*sin(ta1) + la2*sin(ta2));
eqnv_x3 = subs(eqn_x3, sub_var, sub_val);
eqnv_y3 = subs(eqn_y3, sub_var, sub_val);

% Solve for an expression for ta2
if 0
    eqn6 = sym(kx == la2*cos(ta2) - lb2*cos(kz + ta2));
    S = solve(eqn6, ta2);
    expr_ta2 = S(1);
    % It looks like there's an error in this method!
else
    S = solve([eqn1, eqn2], [ta2, tb2]);
    expr_ta2 = S(1).ta2(1);
end

exprv_ta2 = subs(expr_ta2, sub_var, sub_val);

%% Plot (slow, symbolic)

visualizeProcess = 1; % Set to '1' if you want to visualize the plot as it's formed

expr_drawx = sym([xa, xa + la1*cos(ta1), xa + la1*cos(ta1) + la2*cos(ta2), xb + lb1*cos(tb1), xb]);
expr_drawy = sym([ya, ya + la1*sin(ta1), ya + la1*sin(ta1) + la2*sin(ta2), yb + lb1*sin(tb1), yb]);

exprv_drawx = subs(expr_drawx, sub_var, sub_val);
exprv_drawy = subs(expr_drawy, sub_var, sub_val);
    
inca = 0.1;
incb = 0.102;
ta1v = 0;
tb1v = 0;
stepCount = 1000;
vec_x = zeros(stepCount, 1);
vec_y = zeros(stepCount, 1);
vec_ta2 = zeros(stepCount, 1);

for step = 1 : stepCount
    % kx & kz
%     kxv = real(double(subs(exprv_kx, {ta1, tb1}, {ta1v, tb1v})));
%     kzv = real(double(subs(exprv_kz, {ta1, tb1}, {ta1v, tb1v})));
%     ta2v = real(double(subs(exprv_ta2, {kx, kz}, {kxv, kzv})));
    
% kx & ky
    kxv = real(double(subs(exprv_kx, {ta1, tb1}, {ta1v, tb1v})));
    kyv = real(double(subs(exprv_ky, {ta1, tb1}, {ta1v, tb1v})));
    ta2v = real(double(subs(exprv_ta2, {kx, ky}, {kxv, kyv})));
    
    x3 = double(subs(eqnv_x3, {ta1, ta2}, {ta1v, ta2v}));
    y3 = double(subs(eqnv_y3, {ta1, ta2}, {ta1v, ta2v}));
    
    vec_x(step) = x3;
    vec_y(step) = y3; 
    vec_ta2(step) = ta2v;
    
    if visualizeProcess
        drawx = real(double(subs(exprv_drawx, {ta1, ta2, tb1}, {ta1v, ta2v, tb1v})));
        drawy = real(double(subs(exprv_drawy, {ta1, ta2, tb1}, {ta1v, ta2v, tb1v})));
    
        clf

        plot(vec_x(1:step), vec_y(1:step), '-x','LineWidth', 0.5);
        hold on
        plot(drawx, drawy, 'LineWidth', 3);
        title(sprintf('Step=%3d', step))
        xlim([-100, 350])
        ylim([-100, 400])
        drawnow
    else
        if mod(step, 100) == 0
            fprintf('Step=%d\n', step)
        end
    end
          
    % Increment angles
    ta1v = ta1v + inca;
    tb1v = tb1v + incb;
    %pause
end

clf
plot(vec_x, vec_y)
axis square
    
    
%% Plot quick

% Equations:
% ta2 = 2*atan(((2*la2^2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 - (xa - xb + la1*cos(ta1) - lb1*cos(tb1))^4 - (ya - yb + la1*sin(ta1) - lb1*sin(tb1))^4 - 2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 + 2*lb2^2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 + 2*la2^2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2 + 2*lb2^2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2 - la2^4 - lb2^4 + 2*la2^2*lb2^2)^(1/2) - 2*la2*(ya - yb + la1*sin(ta1) - lb1*sin(tb1)))/((xa - xb + la1*cos(ta1) - lb1*cos(tb1))^2 + (ya - yb + la1*sin(ta1) - lb1*sin(tb1))^2 - 2*la2*(xa - xb + la1*cos(ta1) - lb1*cos(tb1)) + la2^2 - lb2^2))
% Simplified to:
% ka = xa - xb + la1*cos(ta1) - lb1*cos(tb1)
% kb = ya - yb + la1*sin(ta1) - lb1*sin(tb1)
% kc = ka^2 + kb^2 - lb2^2
% ta2 = 2*atan((((2*la2*lb2)^2 - (kc - la2^2)^2)^(1/2) - 2*la2*kb)/(kc - 2*la2*ka + la2^2))


visualizeProcess = 0; % Set to '1' if you want to visualize the plot as it's formed
% 
% xa_=0;
% ya_=0;
% xb_=150;
% yb_=0;
% la1_=70;
% lb1_=70;
% la2_=300;
% lb2_=300;

inca = 0.01;
incb = 0.0102;
ta1_ = 0;
tb1_ = 0;
stepCount = 20000;
vec_x = zeros(stepCount, 1);
vec_y = zeros(stepCount, 1);
vec_ta2 = zeros(stepCount, 1);

tic

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
    
    if visualizeProcess && mod(step, 1) == 0
        drawx = [xa_, xa_ + la1_*cos(ta1_), xa_ + la1_*cos(ta1_) + la2_*cos(ta2_), xb_ + lb1_*cos(tb1_), xb_];
        drawy = [ya_, ya_ + la1_*sin(ta1_), ya_ + la1_*sin(ta1_) + la2_*sin(ta2_), yb_ + lb1_*sin(tb1_), yb_];

        clf

        plot(vec_x(1:step), vec_y(1:step), '-','LineWidth', 0.5);
        hold on
        plot(drawx, drawy, 'LineWidth', 3);
        
        title(sprintf('Step=%3d', step))
        xlim([-100, 350])
        ylim([-100, 400])
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
