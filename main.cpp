#include "raylib.h"
#include <vector>
#include <cmath>
#include <fstream>
#include <string>

//_____________________Math Portion ___________________
struct Vec3D {
    double x, y, z;
    Vec3D operator+(const Vec3D& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3D operator-(const Vec3D& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3D operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3D cross(const Vec3D& o) const { return { y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x }; }
    double mag() const { return std::sqrt(x*x + y*y + z*z); }
};

struct State {
    Vec3D pos;
    Vec3D vel;
};

struct TrailPoint {
    Vector3 pos;
    Color color;
};

//_________________PHysics EngiNE________________________
Vec3D getMagneticBottle(const Vec3D& pos, double B0, double gradient) {
    double Bz = B0 * (1.0 + gradient * pos.z * pos.z);
    double Bx = -gradient * B0 * pos.x * pos.z;
    double By = -gradient * B0 * pos.y * pos.z;
    return {Bx, By, Bz};
}

void stepBoris(State& state, double dt, const Vec3D& E, double B0, double gradient, double q, double m) {
    Vec3D B = getMagneticBottle(state.pos, B0, gradient);
    double q_m_half_dt = (q / m) * (dt * 0.5);

    Vec3D v_minus = state.vel + E * q_m_half_dt;
    
    Vec3D t = B * q_m_half_dt;
    double t_mag_sq = (t.x * t.x) + (t.y * t.y) + (t.z * t.z);
    Vec3D s = t * (2.0 / (1.0 + t_mag_sq));

    Vec3D v_prime = v_minus + v_minus.cross(t);
    Vec3D v_plus = v_minus + v_prime.cross(s);

    state.vel = v_plus + E * q_m_half_dt;
    state.pos = state.pos + state.vel * dt;
}

//?_______________________UI_____________________________
double DrawValueControl(int x, int y, const char* label, double value, double step) {
    DrawText(label, x, y, 15, DARKGRAY);
    
    Rectangle minusBtn = {(float)x, (float)y + 20, 30, 25};
    Rectangle plusBtn = {(float)x + 120, (float)y + 20, 30, 25};
    Vector2 mouse = GetMousePosition();
    double change = 0;

    Color mColor = LIGHTGRAY;
    if (CheckCollisionPointRec(mouse, minusBtn)) {
        mColor = GRAY;
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) { mColor = DARKGRAY; change = -step; }
    }
    DrawRectangleRec(minusBtn, mColor);
    DrawText("-", minusBtn.x + 10, minusBtn.y + 5, 20, BLACK);

    Color pColor = LIGHTGRAY;
    if (CheckCollisionPointRec(mouse, plusBtn)) {
        pColor = GRAY;
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) { pColor = DARKGRAY; change = step; }
    }
    DrawRectangleRec(plusBtn, pColor);
    DrawText("+", plusBtn.x + 8, plusBtn.y + 5, 20, BLACK);

    DrawRectangle(x + 35, y + 20, 80, 25, WHITE);
    DrawText(TextFormat("%0.3f", value), x + 40, y + 25, 15, BLACK);

    return value + change;
}

bool DrawTextButton(int x, int y, int width, int height, const char* text, Color baseColor) {
    Rectangle btn = {(float)x, (float)y, (float)width, (float)height};
    Vector2 mouse = GetMousePosition();
    bool clicked = false;
    
    if (CheckCollisionPointRec(mouse, btn)) {
        baseColor = GRAY;
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) baseColor = DARKGRAY;
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) clicked = true;
    }
    
    DrawRectangleRec(btn, baseColor);
    int textW = MeasureText(text, 15);
    DrawText(text, x + (width / 2) - (textW / 2), y + (height / 2) - 7, 15, WHITE);
    return clicked;
}
//------------------------------Main Function +________________________________
int main() {
    State particle = { {0.0, 1.0, 0.0}, {4.0, 0.0, 3.0} };
    double charge = 1.0, mass = 1.0, dt = 0.01; 
    Vec3D E_field = {0.0, 0.0, 0.0}; 
    double B0 = 1.5;         
    double gradient = 0.02;  

    std::vector<TrailPoint> trail;
    bool isPaused = false;
    bool isRecording = false;
    std::ofstream dataLog;
    double simTime = 0.0;

    const int uiWidth = 340; 
    InitWindow(1280, 800, "Advance Lorentz Force & Magnetic Simulator");
    
    // --------------------------- Custom Camera Variables ------------------------(will start at 45 degree both)
    float camRadius = 35.0f;
    float camAngleX = PI / 4.0f; 
    float camAngleY = PI / 4.0f; 

    Camera3D camera = { 0 };
    camera.target = { 0.0f, 0.0f, 0.0f };      
    camera.up = { 0.0f, 1.0f, 0.0f };       
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        ///Rotationm Of camera----------------------
        if (GetMouseX() > uiWidth) { 
            
            
            camRadius -= GetMouseWheelMove() * 2.0f;
            if (camRadius < 2.0f) camRadius = 2.0f;  
            if (camRadius > 100.0f) camRadius = 100.0f; 

            // ----------------------Click and Drag Rotation------------------
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                Vector2 mouseDelta = GetMouseDelta();
                camAngleX -= mouseDelta.x * 0.005f;
                camAngleY -= mouseDelta.y * 0.005f;
                
                // ----------------Prevent camera from flipping upside down-----------------
                if (camAngleY < 0.01f) camAngleY = 0.01f;
                if (camAngleY > PI - 0.01f) camAngleY = PI - 0.01f;
            }
        }

        // Apply Math: Convert Spherical Angles back to X,Y,Z coordinates  IMPORTENT
        camera.position.x = camRadius * sin(camAngleY) * cos(camAngleX);
        camera.position.z = camRadius * sin(camAngleY) * sin(camAngleX);
        camera.position.y = camRadius * cos(camAngleY);
        // ==========================================

        double uiStep = 2.0 * GetFrameTime(); 
        double fineStep = 0.5 * GetFrameTime();

        // --------------ALGORITHm==================
        if (!isPaused) {
            for(int i = 0; i < 5; i++) {
                stepBoris(particle, dt, E_field, B0, gradient, charge, mass);
                simTime += dt;
            }

            double speed = particle.vel.mag();
            double kineticEnergy = 0.5 * mass * (speed * speed);

            float hue = 240.0f - ((float)speed * 12.0f); 
            if (hue < 0.0f) hue = 0.0f; 
            Color trailColor = ColorFromHSV(hue, 1.0f, 1.0f);

            Vector3 renderPos = { (float)particle.pos.x, (float)particle.pos.y, (float)particle.pos.z };
            trail.push_back({renderPos, trailColor});
            if (trail.size() > 3000) trail.erase(trail.begin());

            if (isRecording && dataLog.is_open()) {
                dataLog << simTime << "," << particle.pos.x << "," << particle.pos.y << "," << particle.pos.z << ","
                        << particle.vel.x << "," << particle.vel.y << "," << particle.vel.z << ","
                        << kineticEnergy << "\n";
            }
        }

        // -------------- RENDER SCENE -----------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            BeginMode3D(camera);
                DrawGrid(40, 1.0f);
                DrawLine3D({0,0,0}, {15,0,0}, RED);   
                DrawLine3D({0,0,0}, {0,15,0}, GREEN); 
                DrawLine3D({0,0,0}, {0,0,15}, BLUE);  
                
                for (size_t i = 1; i < trail.size(); i++) {
                    DrawLine3D(trail[i-1].pos, trail[i].pos, trail[i].color);
                }
                
                Vector3 pPos = { (float)particle.pos.x, (float)particle.pos.y, (float)particle.pos.z };
                DrawSphere(pPos, 0.4f, BLACK);
            EndMode3D();

            // ------------------2D UI Panel-------------------
            DrawRectangle(0, 0, uiWidth, 800, Fade(LIGHTGRAY, 0.95f));
            DrawRectangle(uiWidth, 0, 3, 800, DARKGRAY); 

            DrawText("PLASMA PHYSICS ENGINE", 20, 20, 20, BLACK);
            DrawText("Boris Algorithm | Symplectic Integrator", 20, 45, 10, DARKGRAY);
            
            if (DrawTextButton(20, 70, 140, 30, isPaused ? "RESUME" : "PAUSE", isPaused ? MAROON : DARKGREEN)) isPaused = !isPaused;
            if (DrawTextButton(170, 70, 140, 30, "RESET SCENE", DARKBLUE)) {
                particle = { {0.0, 1.0, 0.0}, {4.0, 0.0, 3.0} };
                trail.clear();
                simTime = 0.0;
            }

            if (DrawTextButton(20, 110, 290, 30, isRecording ? "STOP RECORDING DATA" : "START CSV RECORDING", isRecording ? RED : GRAY)) {
                isRecording = !isRecording;
                if (isRecording) {
                    dataLog.open("telemetry_data.csv");
                    dataLog << "Time,PosX,PosY,PosZ,VelX,VelY,VelZ,KineticEnergy\n";
                } else {
                    dataLog.close();
                }
            }
            if (isRecording) DrawCircle(35, 125, 5, RED); 

            DrawText("Particle State:", 20, 170, 15, BLACK);
            charge = DrawValueControl(20, 200, "Charge (q)", charge, uiStep);
            mass = DrawValueControl(170, 200, "Mass (m)", mass, uiStep);
            if (mass < 0.01) mass = 0.01; 

            DrawText("Uniform Electric Field (E):", 20, 260, 15, BLACK);
            E_field.x = DrawValueControl(20, 290, "E-Field X", E_field.x, uiStep);
            E_field.y = DrawValueControl(170, 290, "E-Field Y", E_field.y, uiStep);
            E_field.z = DrawValueControl(20, 350, "E-Field Z", E_field.z, uiStep);

            DrawText("Magnetic Bottle Topology (B):", 20, 420, 15, BLACK);
            B0 = DrawValueControl(20, 450, "Base Field (B0)", B0, fineStep);
            gradient = DrawValueControl(170, 450, "Gradient (a)", gradient, fineStep * 0.1);
            if (gradient < 0.0) gradient = 0.0; 

            DrawRectangle(20, 540, 290, 160, WHITE);
            DrawRectangleLines(20, 540, 290, 160, DARKGRAY);
            DrawText("LIVE TELEMETRY:", 30, 550, 15, DARKGRAY);
            
            Vec3D currentB = getMagneticBottle(particle.pos, B0, gradient);
            
            DrawText(TextFormat("Sim Time:   %.2f s", simTime), 30, 580, 15, BLACK);
            DrawText(TextFormat("Position Z: %.2f m", particle.pos.z), 30, 600, 15, BLACK);
            DrawText(TextFormat("Local Bz:   %.3f T", currentB.z), 30, 620, 15, DARKBLUE);
            DrawText(TextFormat("Speed:      %.2f m/s", particle.vel.mag()), 30, 640, 15, MAROON);
            DrawText(TextFormat("Kinetic E:  %.2f J", 0.5 * mass * particle.vel.mag() * particle.vel.mag()), 30, 660, 15, DARKGREEN);

            DrawText("Rotate Camera: Left Click & Drag", 20, 730, 10, GRAY);
            DrawText("Zoom Camera: Mouse Wheel", 20, 750, 10, GRAY);

        EndDrawing();
    }
    
    if (dataLog.is_open()) dataLog.close();
    CloseWindow();
    return 0;
}
//Code has some portion of Others .........