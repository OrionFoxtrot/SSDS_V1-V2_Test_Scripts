% ChipSat 6DOF drop simulation 
% Conventions
%   Inertial frame I: local tangent frame, +z upward
%   Body frame B:
%       origin at the ChipSat center of mass
%       +x along the 5 cm dimension, toward the right in the mechanical drawing
%       +y along the 6 cm dimension, toward the top in the mechanical drawing
%       +z normal to the plate, out of the drawing
%     This is a right-handed frame: x_B cross y_B = z_B.
%   q_IB = scalar-first Hamilton quaternion that rotates B vectors into I
%   omega_B = body angular velocity resolved in B
%
% This script fixes the rigid-body and inertia issues in the original model and
% replaces the fixed lift/drag/COP damper with a distributed surface-panel model.
% The panel model is still quasi-steady and its coefficients must be calibrated
% against drop-test or CFD data before treating the trajectory as predictive.

clc; clear; close all;

% Vehicle parameters
p.m = 0.040;          % kg
p.Xdim = 0.050;       % m, body x dimension
p.Ydim = 0.060;       % m, body y dimension
p.Zdim = 0.004;       % m, body z dimension / thickness

% Rectangular-prism inertia about the COM and body principal axes.
% Replace this with the CAD inertia tensor for the assembled ChipSat when available.
Ixx = p.m * (p.Ydim^2 + p.Zdim^2) / 12;
Iyy = p.m * (p.Xdim^2 + p.Zdim^2) / 12;
Izz = p.m * (p.Xdim^2 + p.Ydim^2) / 12;
p.I = diag([Ixx, Iyy, Izz]);

% Aerodynamic model parameters
% Normal-pressure coefficients. These are placeholders, not validated values.
p.CnFace = 1.28;      % broad-face normal coefficient
p.CnEdge = 1.10;      % edge-face normal coefficient
p.Cf     = 0.005;     % tangential skin-friction coefficient

% Optional static aerodynamic-center offset from COM.
% Set to zero unless the offset is measured or supported by a detailed model.
% Components are [x_B; y_B; z_B]. The value below is a +0.1 mm y_B offset.
p.rCOP_B = [0; 0.1; 0]/1000;             % m, measured from COM

% Surface discretization [x panels, y panels, thickness panels].
p.Nx = 7;
p.Ny = 7;
p.Nt = 3;
p.panels = buildBoxPanels(p.Xdim, p.Ydim, p.Zdim, ...
    p.Nx, p.Ny, p.Nt, p.CnFace, p.CnEdge);

% Environment
p.wind_I = [0; 0; 0];       % m/s, air velocity in inertial frame
p.g0 = 9.80665;              % m/s^2
p.Re = 6371000;              % m, mean Earth radius
p.qNormGain = 1.0;           % quaternion norm feedback gain, 1/s

% Initial conditions
h0 = 100;                    % m geometric altitude
r0_I = [0; 0; h0];
v0_I = [0; 0; 0];

roll0  = deg2rad(1);
pitch0 = deg2rad(-2);
yaw0   = deg2rad(3);
q0_IB = euler321ToQuat(roll0, pitch0, yaw0);
omega0_B = deg2rad([5; -3; 2]);

% State x = [r_I(3); v_I(3); q_IB(4); omega_B(3)]
x0 = [r0_I; v0_I; q0_IB; omega0_B];

% Quaternion convention check
quatResidual = verifyQuaternionKinematics(q0_IB, deg2rad([17; -23; 31]));
fprintf('Quaternion Rdot residual: %.3e\n', quatResidual);

% Integrate
tspan = [0 2000];
opts = odeset( ...
    'RelTol', 1e-7, ...
    'AbsTol', 1e-9, ...
    'MaxStep', 0.05, ...
    'Events', @groundEvent);

[t, x] = ode45(@(t,x) eomChipSat(t, x, p), tspan, x0, opts);

% Results
r_I = x(:,1:3);
v_I = x(:,4:6);
q_IB = x(:,7:10);
omega_B = x(:,11:13);

% Normalize stored quaternion samples for reporting.
qNorm = vecnorm(q_IB, 2, 2);
q_IB = q_IB ./ qNorm;

altitude = r_I(:,3);
speed = vecnorm(v_I, 2, 2);
bodyRatesDeg = rad2deg(omega_B);

fprintf('Impact time: %.3f s\n', t(end));
fprintf('Impact speed: %.3f m/s\n', speed(end));
fprintf('Maximum body rate: %.3f deg/s\n', ...
    max(vecnorm(bodyRatesDeg, 2, 2)));
fprintf('Maximum quaternion norm error: %.3e\n', max(abs(qNorm - 1)));

% Plots
figure;
plot(t, altitude, 'LineWidth', 1.5);
xlabel('Time (s)');
ylabel('Altitude (m)');
grid on;

figure;
plot(t, speed, 'LineWidth', 1.5);
xlabel('Time (s)');
ylabel('Speed (m/s)');
grid on;

figure;
plot(t, bodyRatesDeg, 'LineWidth', 1.3);
xlabel('Time (s)');
ylabel('Body rate (deg/s)');
legend('p', 'q', 'r', 'Location', 'best');
grid on;

figure;
plot3(r_I(:,1), r_I(:,2), r_I(:,3), 'LineWidth', 1.3);
hold on;
plot3(r0_I(1), r0_I(2), r0_I(3), 'o', ...
    'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'r', ...
    'MarkerSize', 8, 'DisplayName', 'Release');
xlabel('Inertial x (m)');
ylabel('Inertial y (m)');
zlabel('Altitude (m)');
grid on;
axis tight;
view(70,45);
legend('Path', 'Release', 'Location', 'best');

figure;
plot(r_I(:,1), r_I(:,2), 'LineWidth', 1.5);
hold on;
plot(r_I(1,1), r_I(1,2), 'ro', 'MarkerFaceColor', 'r');
plot(r_I(end,1), r_I(end,2), 'kx', 'MarkerSize', 10, ...
    'LineWidth', 2);

xlabel('Inertial x (m)');
ylabel('Inertial y (m)');
title('Horizontal Ground Track');
legend('Path', 'Release', 'Impact', 'Location', 'best');
grid on;
axis equal;

%% Equations of motion
function xdot = eomChipSat(~, x, p)

r_I = x(1:3);
v_I = x(4:6);
qRaw_IB = x(7:10);
omega_B = x(11:13);

% Use a normalized quaternion for coordinate transformations.
q_IB = qRaw_IB / norm(qRaw_IB);
R_IB = quatToDCM(q_IB);       % B vector -> I vector
R_BI = R_IB.';

h = max(r_I(3), 0);
rho = atmosphereDensityUS76(h);
g = p.g0 * (p.Re / (p.Re + h))^2;

% COM velocity relative to the air, resolved in B.
vRelCOM_I = v_I - p.wind_I;
vRelCOM_B = R_BI * vRelCOM_I;

[F_aero_B, M_aero_B] = aeroPanelModel(vRelCOM_B, omega_B, rho, p);

rDot_I = v_I;
vDot_I = [0; 0; -g] + R_IB * F_aero_B / p.m;

% qdot = 1/2 q x [0,omega_B] for a B-to-I quaternion and body-resolved rates.
qDot_IB = quatDerivativeBodyRates(qRaw_IB, omega_B);
qDot_IB = qDot_IB + p.qNormGain * (1 - dot(qRaw_IB,qRaw_IB)) * qRaw_IB;

omegaDot_B = p.I \ (M_aero_B - cross(omega_B, p.I * omega_B));

xdot = [rDot_I; vDot_I; qDot_IB; omegaDot_B];

end

%% Distributed aerodynamic model
function [F_B, M_B] = aeroPanelModel(vRelCOM_B, omega_B, rho, p)

F_B = zeros(3,1);
M_distributed_B = zeros(3,1);

for k = 1:numel(p.panels.area)
    r_B = p.panels.r_B(:,k);
    n_B = p.panels.n_B(:,k);
    dA = p.panels.area(k);
    Cn = p.panels.Cn(k);

    % Local surface-point velocity relative to air.
    u_B = vRelCOM_B + cross(omega_B, r_B);

    % The outward face is windward when u dot n > 0.
    un = dot(u_B, n_B);
    dF_pressure_B = zeros(3,1);
    if un > 0
        % Quasi-steady pressure force based on local normal velocity.
        dF_pressure_B = -0.5 * rho * Cn * dA * un^2 * n_B;
    end

    % Tangential skin drag on the physical surface.
    uTan_B = u_B - un * n_B;
    VTan = norm(uTan_B);
    if VTan > 1e-12
        dF_skin_B = -0.5 * rho * p.Cf * dA * VTan * uTan_B;
    else
        dF_skin_B = zeros(3,1);
    end

    dF_B = dF_pressure_B + dF_skin_B;
    F_B = F_B + dF_B;
    M_distributed_B = M_distributed_B + cross(r_B, dF_B);
end

% Optional static COP shift. The distributed term already supplies
% rotation-dependent aerodynamic damping and moment.
M_B = M_distributed_B + cross(p.rCOP_B, F_B);

end

%% Rectangular-prism panel mesh
function panels = buildBoxPanels(Xdim, Ydim, Zdim, Nx, Ny, Nt, CnFace, CnEdge)

[xc, dx] = midpointGrid(Xdim, Nx);
[yc, dy] = midpointGrid(Ydim, Ny);
[zc, dz] = midpointGrid(Zdim, Nt);

r = zeros(3,0);
n = zeros(3,0);
area = zeros(1,0);
Cn = zeros(1,0);

% Broad faces z_B = +/-Zdim/2.
for s = [-1, 1]
    for ix = 1:Nx
        for iy = 1:Ny
            r(:,end+1) = [xc(ix); yc(iy); s*Zdim/2]; %#ok<AGROW>
            n(:,end+1) = [0; 0; s]; %#ok<AGROW>
            area(end+1) = dx * dy; %#ok<AGROW>
            Cn(end+1) = CnFace; %#ok<AGROW>
        end
    end
end

% x_B-normal edge faces at x_B = +/-Xdim/2.
for s = [-1, 1]
    for iy = 1:Ny
        for iz = 1:Nt
            r(:,end+1) = [s*Xdim/2; yc(iy); zc(iz)]; %#ok<AGROW>
            n(:,end+1) = [s; 0; 0]; %#ok<AGROW>
            area(end+1) = dy * dz; %#ok<AGROW>
            Cn(end+1) = CnEdge; %#ok<AGROW>
        end
    end
end

% y_B-normal edge faces at y_B = +/-Ydim/2.
for s = [-1, 1]
    for ix = 1:Nx
        for iz = 1:Nt
            r(:,end+1) = [xc(ix); s*Ydim/2; zc(iz)]; %#ok<AGROW>
            n(:,end+1) = [0; s; 0]; %#ok<AGROW>
            area(end+1) = dx * dz; %#ok<AGROW>
            Cn(end+1) = CnEdge; %#ok<AGROW>
        end
    end
end

panels.r_B = r;
panels.n_B = n;
panels.area = area;
panels.Cn = Cn;

end

function [c, d] = midpointGrid(lengthValue, count)
d = lengthValue / count;
c = linspace(-lengthValue/2 + d/2, lengthValue/2 - d/2, count);
end

%% U.S. Standard Atmosphere 1976, lower atmosphere approximation
function rho = atmosphereDensityUS76(hGeom)
% Valid through approximately 86 km geometric altitude.
% For higher altitudes, use NRLMSISE-00 or another rarefied-atmosphere model.

if hGeom > 86000
    error(['atmosphereDensityUS76 is intentionally limited to 86 km. ', ...
        'Use NRLMSISE-00 and a rarefied-flow aerodynamic model above this altitude.']);
end

hGeom = max(hGeom, 0);
ReGeo = 6356766;                         % m, geopotential Earth radius
H = ReGeo * hGeom / (ReGeo + hGeom);    % geopotential altitude

Hb = [0, 11000, 20000, 32000, 47000, 51000, 71000, 84852];
Lapse = [-0.0065, 0, 0.0010, 0.0028, 0, -0.0028, -0.0020];

g0 = 9.80665;
Rair = 287.05287;

persistent Tb Pb
if isempty(Tb)
    Tb = zeros(size(Hb));
    Pb = zeros(size(Hb));
    Tb(1) = 288.15;
    Pb(1) = 101325;

    for j = 1:numel(Lapse)
        dH = Hb(j+1) - Hb(j);
        if abs(Lapse(j)) < eps
            Tb(j+1) = Tb(j);
            Pb(j+1) = Pb(j) * exp(-g0 * dH / (Rair * Tb(j)));
        else
            Tb(j+1) = Tb(j) + Lapse(j) * dH;
            Pb(j+1) = Pb(j) * (Tb(j+1)/Tb(j))^(-g0/(Rair*Lapse(j)));
        end
    end
end

layer = find(H >= Hb, 1, 'last');
layer = min(layer, numel(Lapse));
dH = H - Hb(layer);

if abs(Lapse(layer)) < eps
    T = Tb(layer);
    P = Pb(layer) * exp(-g0 * dH / (Rair * T));
else
    T = Tb(layer) + Lapse(layer) * dH;
    P = Pb(layer) * (T/Tb(layer))^(-g0/(Rair*Lapse(layer)));
end

rho = P / (Rair * T);

end

%% Quaternion utilities
function q = euler321ToQuat(roll, pitch, yaw)
% Scalar-first Hamilton quaternion for Rz(yaw)*Ry(pitch)*Rx(roll).

cr = cos(roll/2);  sr = sin(roll/2);
cp = cos(pitch/2); sp = sin(pitch/2);
cy = cos(yaw/2);   sy = sin(yaw/2);

q = [
    cr*cp*cy + sr*sp*sy;
    sr*cp*cy - cr*sp*sy;
    cr*sp*cy + sr*cp*sy;
    cr*cp*sy - sr*sp*cy
    ];
q = q / norm(q);

end

function R = quatToDCM(q)
q = q / norm(q);
qw = q(1); qx = q(2); qy = q(3); qz = q(4);

R = [
    1 - 2*(qy^2 + qz^2),  2*(qx*qy - qw*qz),      2*(qx*qz + qw*qy);
    2*(qx*qy + qw*qz),    1 - 2*(qx^2 + qz^2),    2*(qy*qz - qw*qx);
    2*(qx*qz - qw*qy),    2*(qy*qz + qw*qx),      1 - 2*(qx^2 + qy^2)
    ];
end

function qDot = quatDerivativeBodyRates(q, omega_B)
% qdot = 1/2 * q Hamilton-product [0; omega_B]
qw = q(1); qx = q(2); qy = q(3); qz = q(4);
pRate = omega_B(1);
qRate = omega_B(2);
rRate = omega_B(3);

qDot = 0.5 * [
    -qx*pRate - qy*qRate - qz*rRate;
     qw*pRate + qy*rRate - qz*qRate;
     qw*qRate - qx*rRate + qz*pRate;
     qw*rRate + qx*qRate - qy*pRate
    ];
end

function residual = verifyQuaternionKinematics(q, omega_B)
% For a B-to-I DCM and body-resolved omega, Rdot = R*skew(omega_B).
dT = 1e-7;
qDot = quatDerivativeBodyRates(q, omega_B);
R0 = quatToDCM(q);
R1 = quatToDCM(q + dT*qDot);
RDotFD = (R1 - R0) / dT;
RDotExpected = R0 * skew3(omega_B);
residual = norm(RDotFD - RDotExpected, 'fro');
end

function S = skew3(a)
S = [
     0,    -a(3),  a(2);
     a(3),  0,    -a(1);
    -a(2),  a(1),  0
    ];
end

%% Ground event
function [value, isterminal, direction] = groundEvent(~, x)
value = x(3);
isterminal = 1;
direction = -1;
end
