#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <assert.h>

//----------------------------------------------------------------------------------
// Camera
//----------------------------------------------------------------------------------

// Basic Orbit Camera with simple controls
typedef struct {

    Camera3D cam3d;
    float azimuth;
    float altitude;
    float distance;
    Vector3 offset;

} OrbitCamera;

static inline void OrbitCameraInit(OrbitCamera* camera)
{
    memset(&camera->cam3d, 0, sizeof(Camera3D));
    camera->cam3d.position = (Vector3){ 2.0f, 3.0f, 5.0f };
    camera->cam3d.target = (Vector3){ -0.5f, 1.0f, 0.0f };
    camera->cam3d.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera->cam3d.fovy = 45.0f;
    camera->cam3d.projection = CAMERA_PERSPECTIVE;

    camera->azimuth = 0.0f;
    camera->altitude = 0.4f;
    camera->distance = 4.0f;
    camera->offset = Vector3Zero();
}

static inline void OrbitCameraUpdate(
    OrbitCamera* camera,
    Vector3 target,
    float azimuthDelta,
    float altitudeDelta,
    float offsetDeltaX,
    float offsetDeltaY,
    float mouseWheel,
    float dt)
{
    camera->azimuth = camera->azimuth + 1.0f * dt * -azimuthDelta;
    camera->altitude = Clamp(camera->altitude + 1.0f * dt * altitudeDelta, 0.0, 0.4f * PI);
    camera->distance = Clamp(camera->distance +  20.0f * dt * -mouseWheel, 0.1f, 100.0f);
    
    Quaternion rotationAzimuth = QuaternionFromAxisAngle((Vector3){0, 1, 0}, camera->azimuth);
    Vector3 position = Vector3RotateByQuaternion((Vector3){0, 0, camera->distance}, rotationAzimuth);
    Vector3 axis = Vector3Normalize(Vector3CrossProduct(position, (Vector3){0, 1, 0}));

    Quaternion rotationAltitude = QuaternionFromAxisAngle(axis, camera->altitude);

    Vector3 localOffset = (Vector3){ dt * offsetDeltaX, dt * -offsetDeltaY, 0.0f };
    localOffset = Vector3RotateByQuaternion(localOffset, rotationAzimuth);

    camera->offset = Vector3Add(camera->offset, Vector3RotateByQuaternion(localOffset, rotationAltitude));

    Vector3 cameraTarget = Vector3Add(camera->offset, target);
    Vector3 eye = Vector3Add(cameraTarget, Vector3RotateByQuaternion(position, rotationAltitude));

    camera->cam3d.target = cameraTarget;
    camera->cam3d.position = eye;
}

//----------------------------------------------------------------------------------
// LineFont
//----------------------------------------------------------------------------------

static const char consolines_nums[94] = {
     2,  6,  4, 10, 15, 16,  4,  5,  5,  3,  2,  3,  1,  1,  1, 13,  3,  6,
    11,  4,  8, 13,  2, 14, 15,  2,  4,  2,  2,  2,  7, 28,  3, 12, 10,  9,
     4,  3, 11,  3,  3,  5,  4,  2,  4,  3, 12,  7, 15,  8, 12,  2,  6,  2,
     4,  2,  3,  3,  3,  1,  3,  2,  1,  1, 11, 10,  8, 10, 12,  5, 26,  6,
     4,  6,  4,  3, 11,  6, 10,  9,  9,  5, 10,  5,  6,  2,  4,  2,  4,  3,
    10,  1, 10,  7
};

static const int consolines_offsets[94] = {
      0,   2,   8,  12,  22,  37,  53,  57,  62,  67,  70,  72,  75,  76,
     77,  78,  91,  94, 100, 111, 115, 123, 136, 138, 152, 167, 169, 173,
    175, 177, 179, 186, 214, 217, 229, 239, 248, 252, 255, 266, 269, 272,
    277, 281, 283, 287, 290, 302, 309, 324, 332, 344, 346, 352, 354, 358,
    360, 363, 366, 369, 370, 373, 375, 376, 377, 388, 398, 406, 416, 428,
    433, 459, 465, 469, 475, 479, 482, 493, 499, 509, 518, 527, 532, 542,
    547, 553, 555, 559, 561, 565, 568, 578, 579, 589
};

static const int consolines_lines[596] = {
    0x701F3C1F, 0x2A1F281F, 0x59135206, 0x63175913, 0x6D116317, 0x59305223,
    0x63345930, 0x6D2E6334, 0x563A5608, 0x3B373B05, 0x6A172610, 0x26276A2E,
    0x27222909, 0x2D2F2722, 0x35342D2F, 0x3E313534, 0x510F3E31, 0x590B510F,
    0x620E590B, 0x681A620E, 0x6730681A, 0x19187425, 0x26077038, 0x6D096105,
    0x70156D09, 0x671D7015, 0x5B1B671D, 0x55125B1B, 0x59085512, 0x61055908,
    0x3D263122, 0x40323D26, 0x373A4032, 0x2B38373A, 0x252F2B38, 0x2925252F,
    0x31222925, 0x2538590E, 0x55265F2B, 0x610D590E, 0x6A12610D, 0x6E1C6A12,
    0x6B266E1C, 0x662A6B26, 0x5F2B662A, 0x3F085526, 0x2A282519, 0x290F2519,
    0x2F09290F, 0x37062F09, 0x3F083706, 0x39332A28, 0x46343933, 0x6E1D6621,
    0x5D226621, 0x541A5D22, 0x5210541A, 0x5C1B722B, 0x49165C1B, 0x37164916,
    0x271B3716, 0x102B271B, 0x5C247214, 0x49295C24, 0x37294929, 0x27243729,
    0x10142724, 0x70204620, 0x65324E0D, 0x650E4F32, 0x291F5A1F, 0x42074238,
    0x2423301D, 0x191E2423, 0x1310191E, 0x422D4212, 0x2A1F2C1F, 0x70301A0C,
    0x5A35360A, 0x4A08360A, 0x5F0D4A08, 0x66145F0D, 0x6A206614, 0x652C6A20,
    0x5A35652C, 0x46375A35, 0x31324637, 0x2B2B3132, 0x261F2B2B, 0x2B13261F,
    0x360A2B13, 0x2735270D, 0x6A222722, 0x5E0C6A22, 0x270B2735, 0x502F270B,
    0x5C30502F, 0x672A5C30, 0x6B1D672A, 0x630D6B1D, 0x2625270C, 0x2F2F2625,
    0x38332F2F, 0x40313833, 0x49224031, 0x522C4922, 0x5C2F522C, 0x652B5C2F,
    0x6A1D652B, 0x670E6A1D, 0x49164922, 0x37053739, 0x262B6A2B, 0x6A233705,
    0x6A2B6A23, 0x680F6830, 0x4A0F680F, 0x4A274A0F, 0x270D261F, 0x2E2E261F,
    0x3A322E2E, 0x442F3A32, 0x4A27442F, 0x671E6930, 0x5E12671E, 0x4E0C5E12,
    0x460B4E0C, 0x360C460B, 0x460B4D1D, 0x2914360C, 0x25212914, 0x2A2E2521,
    0x35352A2E, 0x43333535, 0x4C294333, 0x4D1D4C29, 0x6935690A, 0x26156935,
    0x67125C0C, 0x6B206712, 0x662E6B20, 0x5C32662E, 0x522E5C32, 0x3E0E522E,
    0x350B3E0E, 0x2813350B, 0x261F2813, 0x2A2D261F, 0x32332A2D, 0x3C313233,
    0x53103C31, 0x5C0C5310, 0x2920270D, 0x322D2920, 0x3E32322D, 0x4C333E32,
    0x4C334529, 0x5F304C33, 0x67295F30, 0x6A1E6729, 0x66126A1E, 0x5E0B6612,
    0x550A5E0B, 0x4A0D550A, 0x44144A0D, 0x421E4414, 0x4529421E, 0x531F561F,
    0x291F2C1F, 0x531F561F, 0x23242E1E, 0x181E2324, 0x1311181E, 0x420D262E,
    0x5C2D420D, 0x4C354C0B, 0x380A3835, 0x42312610, 0x5C114231, 0x3B1B4A1B,
    0x4D2A4A1B, 0x59304D2A, 0x622D5930, 0x6B22622D, 0x6E146B22, 0x271B2A1B,
    0x0F25122D, 0x0E1C0F25, 0x12100E1C, 0x1F071210, 0x34041F07, 0x4B063404,
    0x5E0C4B06, 0x6C185E0C, 0x70246C18, 0x6D2F7024, 0x65366D2F, 0x563A6536,
    0x483B563A, 0x3538483B, 0x2B333538, 0x292E2B33, 0x2C29292E, 0x37282C29,
    0x2D203728, 0x4F2B3728, 0x53274F2B, 0x54225327, 0x4E195422, 0x41154E19,
    0x32144115, 0x2B173214, 0x291B2B17, 0x2D20291B, 0x6A1F2606, 0x26396A1F,
    0x3832380D, 0x6A0D260D, 0x672B6A0D, 0x490D4924, 0x260D292C, 0x3032292C,
    0x39353032, 0x40323935, 0x49244032, 0x502E4924, 0x5932502E, 0x61305932,
    0x672B6130, 0x26272A35, 0x29182627, 0x30102918, 0x390C3010, 0x460A390C,
    0x560C460A, 0x6112560C, 0x691E6112, 0x6A27691E, 0x66356A27, 0x690A260A,
    0x6920690A, 0x260A2620, 0x632C6920, 0x5933632C, 0x49375933, 0x39344937,
    0x2E2E3934, 0x26202E2E, 0x260F2632, 0x690F260F, 0x6932690F, 0x490F4931,
    0x6A102510, 0x6A326A10, 0x4810482F, 0x48354823, 0x28354835, 0x6A276635,
    0x67196A27, 0x5D0E6719, 0x4A085D0E, 0x3C094A08, 0x300E3C09, 0x2819300E,
    0x26252819, 0x28352625, 0x6B0A240A, 0x4936490A, 0x24366B36, 0x2633260C,
    0x6933690C, 0x261F691F, 0x692D690E, 0x312D692D, 0x2A25312D, 0x261B2A25,
    0x2A0D261B, 0x6A0D250D, 0x49176A32, 0x25334917, 0x490D4917, 0x26126A12,
    0x26342612, 0x6A0B2507, 0x3F1F6A0B, 0x6A353F1F, 0x25386A35, 0x6B0C250C,
    0x24356B0C, 0x6A352435, 0x662D6A20, 0x5E33662D, 0x4C385E33, 0x39354C38,
    0x2C2E3935, 0x26202C2E, 0x28142620, 0x340A2814, 0x4707340A, 0x580A4707,
    0x6613580A, 0x6A206613, 0x6A0D260D, 0x420D4226, 0x4A304226, 0x56354A30,
    0x632F5635, 0x6A1F632F, 0x6A0D6A1F, 0x122B153A, 0x1E1F122B, 0x251F1E1F,
    0x2B2D251F, 0x3C362B2D, 0x49383C36, 0x59354938, 0x662D5935, 0x6A20662D,
    0x64116A20, 0x560A6411, 0x4707560A, 0x360A4707, 0x2A12360A, 0x251F2A12,
    0x6A0D250D, 0x6A206A0D, 0x652C6A20, 0x5A31652C, 0x512F5A31, 0x512F4822,
    0x480D4822, 0x25344822, 0x6A20682F, 0x67136A20, 0x610D6713, 0x5A0B610D,
    0x520E5A0B, 0x4A1A520E, 0x412E4A1A, 0x3D32412E, 0x35343D32, 0x2D303534,
    0x26222D30, 0x280A2622, 0x69396908, 0x261F691F, 0x340B6A0B, 0x6A353435,
    0x2A10340B, 0x251F2A10, 0x292B251F, 0x3435292B, 0x251F6A06, 0x6A3A251F,
    0x260E6A06, 0x511F260E, 0x2634511F, 0x6A382634, 0x6A352508, 0x6A0A2536,
    0x40206A06, 0x6A394020, 0x25204020, 0x6936690B, 0x26096936, 0x26362609,
    0x7117712C, 0x0F177117, 0x0F2C0F17, 0x1933700E, 0x71287112, 0x0F287128,
    0x0F130F28, 0x6A1F4B0C, 0x4B336A1F, 0x0F3C0F03, 0x681D7013, 0x560F5920,
    0x4E312531, 0x562C4E31, 0x5920562C, 0x3F153F31, 0x370D3F15, 0x2F0B370D,
    0x26112F0B, 0x251B2611, 0x2825251B, 0x32312825, 0x290D6F0D, 0x251C290D,
    0x2728251C, 0x36332728, 0x43353633, 0x50314335, 0x562C5031, 0x5924562C,
    0x551B5924, 0x4B0D551B, 0x59255632, 0x55185925, 0x4A0F5518, 0x3F0C4A0F,
    0x330F3F0C, 0x2819330F, 0x26252819, 0x29322625, 0x25317031, 0x27213731,
    0x25192721, 0x29112519, 0x310C2911, 0x3E0A310C, 0x4E0E3E0A, 0x55154E0E,
    0x59215515, 0x53315921, 0x4035400B, 0x4B334035, 0x532F4B33, 0x5728532F,
    0x59205728, 0x56155920, 0x4C0D5615, 0x400B4C0D, 0x320D400B, 0x2815320D,
    0x25242815, 0x28332524, 0x661B261B, 0x6E397027, 0x6D20661B, 0x70276D20,
    0x4F074F37, 0x58285837, 0x512E5828, 0x591C5828, 0x5411591C, 0x4A0D5411,
    0x3D124A0D, 0x391E3D12, 0x370E3D12, 0x310C370E, 0x2B0F310C, 0x28142B0F,
    0x282A2814, 0x220C2814, 0x2532282A, 0x21352532, 0x1C362135, 0x16331C36,
    0x122D1633, 0x0E1E122D, 0x110F0E1E, 0x150A110F, 0x1B09150A, 0x220C1B09,
    0x3C2A391E, 0x47303C2A, 0x512E4730, 0x250D700D, 0x25324F32, 0x5519490D,
    0x59235519, 0x562D5923, 0x4F32562D, 0x2634260D, 0x57212621, 0x570E5721,
    0x6A1F6D1F, 0x6A296D29, 0x572B570D, 0x1E2B572B, 0x0E16110A, 0x11220E16,
    0x1E2B1122, 0x250F700F, 0x411B410F, 0x5932411B, 0x2533411B, 0x2633260D,
    0x6F212621, 0x6F0E6F21, 0x25085808, 0x25204C20, 0x25365136, 0x56134808,
    0x59195613, 0x551E5919, 0x4C20551E, 0x572A4C20, 0x5930572A, 0x57345930,
    0x51365734, 0x580D250D, 0x25324E32, 0x54174A0D, 0x59245417, 0x542F5924,
    0x4E32542F, 0x500E3E09, 0x591D500E, 0x552E591D, 0x4834552E, 0x35354834,
    0x292C3535, 0x251F292C, 0x2A11251F, 0x330B2A11, 0x3E09330B, 0x570D0E0D,
    0x571C490D, 0x5925571C, 0x54305925, 0x43355430, 0x31324335, 0x28283132,
    0x251D2828, 0x290D251D, 0x0F315531, 0x591F5531, 0x5312591F, 0x400A5312,
    0x2E0D400A, 0x27132E0D, 0x251B2713, 0x2B26251B, 0x37312B26, 0x570F250F,
    0x571F480F, 0x5928571F, 0x55325928, 0x49355532, 0x5921572F, 0x56145921,
    0x4D0F5614, 0x44134D0F, 0x3B2B4413, 0x36303B2B, 0x30323630, 0x292C3032,
    0x251D292C, 0x280D251D, 0x69173317, 0x2A1B3317, 0x26272A1B, 0x27342627,
    0x58345806, 0x2F0D580D, 0x26325932, 0x28132F0D, 0x251B2813, 0x2A26251B,
    0x34322A26, 0x251F5809, 0x5835251F, 0x25115806, 0x4A1F2511, 0x24314A1F,
    0x58382431, 0x5833250B, 0x580D2534, 0x171A5935, 0x1113171A, 0x10071113,
    0x241F580A, 0x5831580D, 0x260C5831, 0x2634260C, 0x6F24712F, 0x671D6F24,
    0x4E1D671D, 0x48194E1D, 0x430C4819, 0x3F19430C, 0x371E3F19, 0x1B1E371E,
    0x13221B1E, 0x0F2F1322, 0x0F1F7C1F, 0x6F1A710F, 0x67216F1A, 0x4E216721,
    0x48254E21, 0x43324825, 0x3F254332, 0x37203F25, 0x1B203720, 0x131C1B20,
    0x0F0F131C, 0x450A3D08, 0x4A13450A, 0x431E4A13, 0x3B27431E, 0x392D3B27,
    0x3F35392D, 0x47373F35
};

void DrawDebugText3D(
    const char* string, 
    Vector3 location, 
    Quaternion rotation, 
    Color color,
    float thick,      // Thickness. Set to 0.0f for pixel lines.
    float scale,      // Overall scale. Default to 1.0f
    float width,      // Width. Default to 1.0f
    float height,     // Height. Default to 1.0f
    float spacing,    // Character Spacing. Default to 0.0f
    float lineheight) // Line Spacing. Default to 1.0f
{
    int slen = strlen(string);
  
    float xOffset = 0.0;
    float yOffset = 0.0;
  
    for (int i = 0; i < slen; i++)
    {
        if (string[i] == '\n')
        {
            // If we have a line break then reset xOffset and move to next line
            xOffset = 0.0;
            yOffset -= lineheight * scale * height;
        }
        else if (string[i] == '\r')
        {
            // Carriage return resets the xOffset
            xOffset = 0.0;
        }
        else if (string[i] == '\t')
        {
            // Tab is like four spaces
            xOffset += 4.0f * 0.5f * scale * width + spacing;          
        }
        else if (string[i] < '!' || string[i] > '~')
        {
            // Every other non-printable character is rendered as a space
            xOffset += 0.5f * scale * width + spacing;
        }
        else
        {
            // Get the look-up table index from the character code
            int ci = string[i] - '!';
            
            // Get the number of line segments and the offset into the lines array
            int lnum = consolines_nums[ci];
            int loff = consolines_offsets[ci];
            
            for (int li = 0; li < lnum; li++)
            {
                // Unpack integer coordinates
                int xStartInt = (0x000000FF & (consolines_lines[loff + li] >> 0));
                int yStartInt = (0x000000FF & (consolines_lines[loff + li] >> 8));
                int xEndInt = (0x000000FF & (consolines_lines[loff + li] >> 16));
                int yEndInt = (0x000000FF & (consolines_lines[loff + li] >> 24));
              
                // Compute the line start position in 3D space
                Vector3 start = (Vector3){ 
                    xOffset + scale *  width * (       ((float)xStartInt) / 128),
                    yOffset - scale * height * (1.0f - ((float)yStartInt) / 128),
                    0.0f };
              
                start = Vector3Add(
                    Vector3RotateByQuaternion(start, rotation), location);
                
                // Compute the line end position in 3D space
                Vector3 end = (Vector3){ 
                    xOffset + scale *  width * (       ((float)xEndInt) / 128),
                    yOffset - scale * height * (1.0f - ((float)yEndInt) / 128),
                    0.0f };
                
                end = Vector3Add(
                    Vector3RotateByQuaternion(end, rotation), location);
                
                // If thickness is zero draw as a line otherwise draw as a capsule
                if (thick == 0.0f)
                {
                    DrawLine3D(start, end, color);
                }
                else
                {
                    DrawCapsule(start, end, thick, 5, 2, color);                  
                }
            }
            
            // Shift forward to next location
            xOffset += 0.5f * scale * width + spacing;
        }
    }
}

//----------------------------------------------------------------------------------
// App
//----------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;
    
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Consolines");
    SetTargetFPS(60);
    
    // Camera
    
    OrbitCamera camera;
    OrbitCameraInit(&camera);
    
    // Drawing properties
    
    float thickness = 0.025;
    float scale = 1.0f;
    float width = 1.0f;
    float height = 1.0f;
    float spacing = 0.0;
    float lineheight = 1.0f;
    
    // Go
    
    while (!WindowShouldClose())
    {
        // Update Camera
        
        OrbitCameraUpdate(
            &camera,
            (Vector3){ 0.0f, 1.5f, 0.0f },
            (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(0)) ? GetMouseDelta().x : 0.0f,
            (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(0)) ? GetMouseDelta().y : 0.0f,
            (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(1)) ? GetMouseDelta().x : 0.0f,
            (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(1)) ? GetMouseDelta().y : 0.0f,
            GetMouseWheelMove(),
            GetFrameTime());
        
        // Draw Text
        
        BeginDrawing();

        BeginMode3D(camera.cam3d);
        
        ClearBackground(RAYWHITE);
        
        const char* testSentence = 
            "the quick brown fox jumps over the lazy dog\n"
            "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n"
            "!\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`{|}~";
        
        DrawDebugText3D(
            testSentence,
            (Vector3){ -10.0f, 3.1f, 0.0f },
            QuaternionIdentity(),
            RED,
            thickness,
            scale,
            width,
            height,
            spacing,
            lineheight);
        
        DrawGrid(10, 1.0f);
        
        EndMode3D();

        // UI
      
        GuiSliderBar((Rectangle){ 80, 10, 80, 20 }, "thickness", TextFormat("%4.2f", thickness), &thickness, 0.0f, 0.1f);
        GuiSliderBar((Rectangle){ 80, 40, 80, 20 }, "scale", TextFormat("%4.2f", scale), &scale, 0.0f, 2.0f);
        GuiSliderBar((Rectangle){ 280, 10, 80, 20 }, "width", TextFormat("%4.2f", width), &width, 0.0f, 2.0f);
        GuiSliderBar((Rectangle){ 280, 40, 80, 20 }, "height", TextFormat("%4.2f", height), &height, 0.0f, 2.0f);
        GuiSliderBar((Rectangle){ 480, 10, 80, 20 }, "spacing", TextFormat("%4.2f", spacing), &spacing, 0.0f, 1.0f);
        GuiSliderBar((Rectangle){ 480, 40, 80, 20 }, "lineheight", TextFormat("%4.2f", lineheight), &lineheight, 0.0f, 2.0f);
        
        EndDrawing();
    }

    CloseWindow();

    return 0;
}