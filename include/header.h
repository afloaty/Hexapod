#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include <SoftwareSerial.h>

#define SERVO
//#define SERVO_CALIBRATION

class Vector3
{
public:
    float x, y, z;

    Vector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f);
    Vector3 operator+(const Vector3 &other) const;
    Vector3 operator-(const Vector3 &other) const;
    Vector3 operator*(const float &scalar) const;
    Vector3 operator/(const float &scalar) const;
    Vector3 &operator=(const Vector3 &other);
    bool operator==(const Vector3 &other) const;
    bool operator!=(const Vector3 &other) const;
    Vector3 rotate(const float &angle, const char &axis); // !! currently not working !! return a rotated vector (Read Only)
    Vector3 normalized();                                 // return a normalized vector (Read Only)
    float magnitude() const;                              // return the length of a vector (Read Only)
    Vector3 inverse() const;                              // return the inverse of the vector (Read Only)
    // Vector2 toVector2() const;                            // return the x & y components of a Vector3 as Vector2 (Read Only)
    Vector3 xyPlane() const; // return the vector but with z = 0 (Read Only)

    static Vector3 Lerp(const Vector3 &start, const Vector3 &end, const float &t); // Linearly interpolate between two points (Read Only)
    static Vector3 Normalize(Vector3 &vector);                                     // normalize the input vector
    static float Dot(const Vector3 &a, const Vector3 &b);                          // return dot product of two vectors
    static float Angle(const Vector3 &from, const Vector3 &to);                    // !! currently only works with angles bellow 90° !! return the angle between two vectors (Read Only)
    static Vector3 ClampMagnitude(Vector3 &vector, const float &magnitude);        // clamp the length of a vector but keeps the direction

    static Vector3 forward; // Shorthand for writing Vector3(1, 0, 0)
    static Vector3 back;    // Shorthand for writing Vector3(-1, 0, 0)
    static Vector3 right;   // Shorthand for writing Vector3(0, 1, 0)
    static Vector3 left;    // Shorthand for writing Vector3(0, -1, 0)
    static Vector3 up;      // Shorthand for writing Vector3(0, 0, 1)
    static Vector3 down;    // Shorthand for writing Vector3(0, 0, -1)
    static Vector3 zero;    // Shorthand for writing Vector3(0, 0, 0)
};

class Vector2
{
public:
    float x, y;

    Vector2(float _x = 0.0f, float _y = 0.0f);
    Vector2 operator+(const Vector2 &other) const;
    Vector2 operator-(const Vector2 &other) const;
    Vector2 operator*(const float &scalar) const;
    Vector2 operator/(const float &scalar) const;
    Vector2 &operator=(const Vector2 &other);
    bool operator==(const Vector2 &other) const;
    bool operator!=(const Vector2 &other) const;
    Vector2 rotate(const float &angle); // return a rotated vector (Read Only)
    Vector2 normalized() const;         // return a normalized vector (Read Only)
    float magnitude() const;            // return the length of the vector (Read Only)
    Vector2 inverse() const;            // return the inverse of the vector (Read Only)
    Vector3 toVector3() const;          // return the x & y components of a Vector2 as Vector3 with z = 0 (Read Only)

    static Vector2 Lerp(const Vector2 &start, const Vector2 &end, const float &t); // Linearly interpolate between two points (Read Only)
    static Vector2 Normalize(Vector2 &vector);                                     // normalize the input vector
    static float Dot(const Vector2 &a, const Vector2 &b);                          // return dot product of two vectors
    static float Angle(const Vector2 &from, const Vector2 &to);                    // return the angle between two vectors (Read Only)
    static Vector2 ClampMagnitude(Vector2 &vector, const float &magnitude);        // clamp the length of a vector but keeps the direction

    static Vector2 forward; // Shorthand for writing Vector2(1, 0)
    static Vector2 back;    // Shorthand for writing Vector2(-1, 0)
    static Vector2 right;   // Shorthand for writing Vector2(0, 1)
    static Vector2 left;    // Shorthand for writing Vector2(0, -1)
    static Vector2 zero;    // Shorthand for writing Vector2(0, 0)
};

bool almostEqual(const float &a, const float &b, const float &epsilon = 0.005);
float calculatePathLength(const std::vector<Vector3> &path);
size_t findLongestPath(const std::vector<float> &pathLengths);
Vector3 interpolatePathByLength(const std::vector<Vector3> &path, float targetLength);
Vector2 projectPointToCircle(const float &radius, const Vector2 &point, Vector2 direction);

size_t findSmallestValue(const std::vector<float> &array); // return index of smallest value in the array

// ================================================================
// ===                           main                           ===
// ================================================================
extern int loopTime;
extern Vector2 directionInput; // what direction and how fast (vector length) the hexapod should walking
extern float rotationInput;    // how much and how fast should the hexapod rotate
extern float groundClearance;  // how high off the ground is the hexapod walking
extern float stepRadius;       // how far of a step can be taken
extern float maxSpeed;         // maximum speed for leg movement
extern float speedMultiplier;  // Speed multiplier (0.0 to 2.0, where 1.0 is default)

// ================================================================
// ===                          output                          ===
// ================================================================

#ifdef SERVO

#include <Adafruit_PWMServoDriver.h>

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

#define SERVO_MIN 100
#define SERVO_MAX 450

#define SERVO_BASELINE 100

struct Servo_Struct
{
    int ch = 0;                        // channel on servo driver
    int minAngle = 50, maxAngle = 130; // max and min angle with 90° being the center position
    int targetAngle = 90;              // Servo angle from 0° to 180°
    int angleOffset = 0;               // if the servos are not installed correctly ((0,0,0) position does not form right angle) an offset can be applied
};

struct Leg_Struct
{
    Servo_Struct Servo[3];            // servos for each leg (from root to tip)
    bool mirrored;                    // ik for mirrored legs is slightly different
    int mountAngle;                   // angle at which the leg is mounted (from center line forwared)
    Vector3 targetPosition;           // endpoint the leg is trying to reach (used for IK calculations)
    Vector3 curPosition;              // current enpoint of leg
    std::vector<Vector3> pointOnPath; // dynamic number of points on a path that the is traveling
    bool lifted = false;              // if the leg is lifted for taking a step -> set to true
};

extern Leg_Struct Leg[6];

void Servo_init();

void Servo_update(const Servo_Struct &servo, const int &onValue = 0);
void Servo_update(const int &servoCH, const int &angle, const int &onValue = 0);

void Servo_deactivateAll();

void Servo_moveAllToMinValue();
void Servo_moveAllToMaxValue();

void Leg_update(const int &legID);
void Output_update();
void setAllServosToNeutral();

#endif

// ================================================================
// ===                            IK                            ===
// ================================================================

#ifdef SERVO

#define LENGTH_COXA 50
#define LENGTH_FEMUR 80
#define LENGTH_TIBIA 130

void calcLegServoAngles(Leg_Struct &leg);

#endif

// ================================================================
// ===                         walkGait                         ===
// ================================================================

enum State
{
    WALKING,
    SITTING,
    STANDING
};

extern State HexapodState;

#ifdef SERVO

extern int legLiftDistance;  // how high each step of the ground is
extern float legLiftIncline; // how steep the incline of leg ascent when lifting is (when moving forward)

void standUp();
void sitDown();
void walkCycle();

#endif

// ================================================================
// ===                         Wifi                         ===
// ================================================================
#include <WebServer.h>
#include <WiFi.h>

// Web server on port 80
extern WebServer server;

// WiFi credentials
extern const char* ssid;
extern const char* password;

// HTML page declaration
extern const char* htmlPage;

// LED pin
const int ledPin = 2;


