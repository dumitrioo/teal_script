#pragma once

#include <commondefs.hpp>

#include <raylib.h>
#include <raymath.h>

#include <scaflux_value.hpp>
#include <scaflux_util.hpp>
#include <scaflux_interfaces.hpp>

class ray_ext: public scfx::extension_interface {
    scfx::valbox color_val(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
        return scfx::valbox{Color{r, g, b, a}, "Color"};
    }

public:
    ray_ext() = default;
    ~ray_ext() {
        unregister_runtime();
    }
    ray_ext(ray_ext const &) = delete;
    ray_ext &operator=(ray_ext const &) = delete;
    ray_ext(ray_ext &&) = delete;
    ray_ext &operator=(ray_ext &&) = delete;

    void register_runtime(scfx::runtime_interface *rt) override {
        std::unique_lock l{rt_mtp_};
        if(rt_ != nullptr) {
            return;
        }
        rt_ = rt;
        if(rt_ == nullptr) {
            return;
        }
        rt->add_function("Vector2", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(2)
            return scfx::valbox{
                Vector2{
                    args.size() > 0 ? SCFXNUMARG(0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(1, float) : 0
                },
                "Vector2"
            };
        });
        rt->add_method("Vector2", "x", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).x; });
        rt->add_method("Vector2", "y", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).y; });
        rt->add_method("Vector2", "set_x", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).x = SCFXNUMARG(1, float); return {}; });
        rt->add_method("Vector2", "set_y", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).y = SCFXNUMARG(1, float); return {}; });

        rt->add_function("Vector3", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(3)
            return scfx::valbox{
                Vector3{
                    args.size() > 0 ? SCFXNUMARG(0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(1, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(2, float) : 0
                },
                "Vector3"
            };
        });
        rt->add_method("Vector3", "x", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).x; });
        rt->add_method("Vector3", "y", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).y; });
        rt->add_method("Vector3", "z", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).y; });
        rt->add_method("Vector3", "set_x", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).x = SCFXNUMARG(1, float); return {}; });
        rt->add_method("Vector3", "set_y", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).y = SCFXNUMARG(1, float); return {}; });
        rt->add_method("Vector3", "set_z", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).y = SCFXNUMARG(1, float); return {}; });

        rt->add_function("Vector4", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
            return scfx::valbox{
                Vector4{
                    args.size() > 0 ? SCFXNUMARG(0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(1, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(2, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_function("Quaternion", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
            return scfx::valbox{
                Vector4{
                    args.size() > 0 ? SCFXNUMARG(0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(1, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(2, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_method("Vector4", "x", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).x; });
        rt->add_method("Vector4", "y", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).y; });
        rt->add_method("Vector4", "z", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).y; });
        rt->add_method("Vector4", "w", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Vector2).y; });
        rt->add_method("Vector4", "set_x", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).x = SCFXNUMARG(1, float); return {}; });
        rt->add_method("Vector4", "set_y", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).y = SCFXNUMARG(1, float); return {}; });
        rt->add_method("Vector4", "set_z", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).y = SCFXNUMARG(1, float); return {}; });
        rt->add_method("Vector4", "set_w", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SCFXTHIS(Vector2 &).y = SCFXNUMARG(1, float); return {}; });

        rt->add_function("Color", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
            return scfx::valbox{
                Color{
                    args.size() > 0 ? SCFXNUMARG(0, uint8_t) : (uint8_t)0,
                    args.size() > 1 ? SCFXNUMARG(1, uint8_t) : (uint8_t)0,
                    args.size() > 2 ? SCFXNUMARG(2, uint8_t) : (uint8_t)0,
                    args.size() > 3 ? SCFXNUMARG(3, uint8_t) : (uint8_t)255
                },
                "Color"
            };
        });
        rt->add_method("Color", "r", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Color).r; });
        rt->add_method("Color", "g", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Color).g; });
        rt->add_method("Color", "b", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Color).b; });
        rt->add_method("Color", "a", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SCFXTHIS(Color).a; });
        rt->add_method("Color", "set_r", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) SCFXTHIS(Color &).r = SCFXNUMARG(1, uint8_t); return {}; });
        rt->add_method("Color", "set_g", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) SCFXTHIS(Color &).g = SCFXNUMARG(1, uint8_t); return {}; });
        rt->add_method("Color", "set_b", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) SCFXTHIS(Color &).b = SCFXNUMARG(1, uint8_t); return {}; });
        rt->add_method("Color", "set_a", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) SCFXTHIS(Color &).a = SCFXNUMARG(1, uint8_t); return {}; });

        rt->add_var("LIGHTGRAY",  color_val(200, 200, 200));
        rt->add_var("GRAY",       color_val(130, 130, 130));
        rt->add_var("DARKGRAY",   color_val(80, 80, 80));
        rt->add_var("YELLOW",     color_val(253, 249, 0));
        rt->add_var("GOLD",       color_val(255, 203, 0));
        rt->add_var("ORANGE",     color_val(255, 161, 0));
        rt->add_var("PINK",       color_val(255, 109, 194));
        rt->add_var("RED",        color_val(230, 41, 55));
        rt->add_var("MAROON",     color_val(190, 33, 55));
        rt->add_var("GREEN",      color_val(0, 228, 48));
        rt->add_var("LIME",       color_val(0, 158, 47));
        rt->add_var("DARKGREEN",  color_val(0, 117, 44));
        rt->add_var("SKYBLUE",    color_val(102, 191, 255));
        rt->add_var("BLUE",       color_val(0, 121, 241));
        rt->add_var("DARKBLUE",   color_val(0, 82, 172));
        rt->add_var("PURPLE",     color_val(200, 122, 255));
        rt->add_var("VIOLET",     color_val(135, 60, 190));
        rt->add_var("DARKPURPLE", color_val(112, 31, 126));
        rt->add_var("BEIGE",      color_val(211, 176, 131));
        rt->add_var("BROWN",      color_val(127, 106, 79));
        rt->add_var("DARKBROWN",  color_val(76, 63, 47));
        rt->add_var("WHITE",      color_val(255, 255, 255));
        rt->add_var("BLACK",      color_val(0, 0, 0));
        rt->add_var("BLANK",      color_val(0, 0, 0));
        rt->add_var("MAGENTA",    color_val(255, 0, 255));
        rt->add_var("RAYWHITE",   color_val(245, 245, 245));


        rt->add_function("ray_get_screen_width", SCFXFUN(/*fname*/, /*args*/) { return GetScreenWidth(); });
        rt->add_function("ray_get_screen_height", SCFXFUN(/*fname*/, /*args*/) { return GetScreenHeight(); });
        rt->add_function("ray_get_monitor_count", SCFXFUN(/*fname*/, /*args*/) { return GetMonitorCount(); });
        rt->add_function("ray_get_current_monitor", SCFXFUN(/*fname*/, /*args*/) { return GetCurrentMonitor(); });
        rt->add_function("ray_get_monitor_width", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return GetMonitorWidth(SCFXNUMARG(0, int)); });
        rt->add_function("ray_get_monitor_height", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return GetMonitorHeight(SCFXNUMARG(0, int)); });


        rt->add_function("ray_init_window", SCFXFUN(/*fname*/, args) {
            InitWindow(
                args.size() > 0 ? SCFXNUMARG(0, int) > 0 ? SCFXNUMARG(0, int) : 800 : 800,
                args.size() > 1 ? SCFXNUMARG(1, int) > 0 ? SCFXNUMARG(1, int) : 600 : 600,
                args.size() > 2 ? args[2].cast_to_string().c_str() : "Raylib Window"
            );
            return scfx::valbox{};
        });
        rt->add_function("ray_window_should_close", SCFXFUN(/*fname*/, /*args*/) { return WindowShouldClose(); });
        rt->add_function("ray_begin_drawing", SCFXFUN(/*fname*/, /*args*/) { BeginDrawing(); return scfx::valbox{}; });
        rt->add_function("ray_end_drawing", SCFXFUN(/*fname*/, /*args*/) { EndDrawing(); return scfx::valbox{}; });
        rt->add_function("ray_close_window", SCFXFUN(/*fname*/, /*args*/) { CloseWindow(); return scfx::valbox{}; });
        rt->add_function("ray_clear_background", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(1)
            ClearBackground(SCFXTHIS(Color));
            return scfx::valbox{};
        });

        // SetTargetFPS
        rt->add_function("ray_set_target_fps", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(1)
            SetTargetFPS(args[0].cast_to_s32());
            return scfx::valbox{};
        });

        rt->add_function("ray_toggle_full_screen", SCFXFUN(/*fname*/, /*args*/) { ToggleFullscreen(); return scfx::valbox{}; });

        rt->add_function("ray_draw_pixel", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(3)
            DrawPixel(SCFXNUMARG(0, int), SCFXNUMARG(1, int), SCFXCLASSARG(2, Color));
            return scfx::valbox{};
        });
        rt->add_function("ray_draw_pixel_v", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(2)
            DrawPixelV(SCFXTHIS(Vector2), SCFXCLASSARG(1, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_draw_line", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(5)
            DrawLine(SCFXNUMARG(0, int), SCFXNUMARG(1, int), SCFXNUMARG(2, int), SCFXNUMARG(3, int), SCFXCLASSARG(4, Color)); return scfx::valbox{};
        });
        rt->add_function("ray_draw_line_v", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(3)
            DrawLineV(SCFXTHIS(Vector2), SCFXCLASSARG(1, Vector2), SCFXCLASSARG(2, Color)); return scfx::valbox{};
        });
        rt->add_function("ray_draw_line_ex", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
            DrawLineEx(SCFXTHIS(Vector2), SCFXCLASSARG(1, Vector2), SCFXNUMARG(2, float), SCFXCLASSARG(3, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_draw_line_strip", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(2)
            auto &&vec2arr{args[0].as_array()};
            std::vector<Vector2> pts{};
            pts.reserve(vec2arr.size());
            for(auto &&v: vec2arr) {
                pts.push_back(v.as_class<Vector2>());
            }
            DrawLineStrip(pts.data(), pts.size(), SCFXCLASSARG(1, Color));
            return {};
        });


        rt->add_function("ray_draw_line_bezier", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
            DrawLineBezier(SCFXTHIS(Vector2), SCFXCLASSARG(1, Vector2), SCFXNUMARG(2, int), SCFXCLASSARG(3, Color)); return scfx::valbox{};
        });
        rt->add_function("ray_draw_circle", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(4)
            DrawCircle(SCFXNUMARG(0, int), SCFXNUMARG(1, int), SCFXNUMARG(2, float), SCFXCLASSARG(3, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_draw_text", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(5)
            DrawText(
                args[0].cast_to_string().c_str(),
                SCFXNUMARG(1, int), SCFXNUMARG(2, int),
                SCFXNUMARG(3, int),
                SCFXCLASSARG(4, Color)
            );
            return scfx::valbox{};
        });

        rt->add_function("ray_draw_rectangle", SCFXFUN(/*fname*/, args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(5)
            DrawRectangle(SCFXNUMARG(0, int), SCFXNUMARG(1, int), SCFXNUMARG(2, int), SCFXNUMARG(3, int), SCFXCLASSARG(4, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_is_key_pressed", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsKeyPressed(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_key_pressed_repeat", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsKeyPressedRepeat(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_key_down", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsKeyDown(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_key_released", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsKeyReleased(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_key_up", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsKeyUp(SCFXNUMARG(0, int)); });
        rt->add_function("ray_get_key_pressed", SCFXFUN(/*fname*/, /*args*/) { return GetKeyPressed(); });
        rt->add_function("ray_get_char_pressed", SCFXFUN(/*fname*/, /*args*/) { return GetCharPressed(); });
        rt->add_function("ray_set_exit_key", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) SetExitKey(SCFXNUMARG(0, int)); return {}; });
        rt->add_function("ray_is_gamepad_available", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsGamepadAvailable(SCFXNUMARG(0, int)); });
        rt->add_function("ray_get_gamepad_name", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return std::string{GetGamepadName(SCFXNUMARG(0, int))}; });
        rt->add_function("ray_is_gamepad_button_pressed", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) return IsGamepadButtonPressed(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); });
        rt->add_function("ray_is_gamepad_button_down", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) return IsGamepadButtonDown(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); });
        rt->add_function("ray_is_gamepad_button_released", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) return IsGamepadButtonReleased(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); });
        rt->add_function("ray_is_gamepad_button_up", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) return IsGamepadButtonUp(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); });
        rt->add_function("ray_get_gamepad_button_pressed", SCFXFUN(/*fname*/, /*args*/) { return GetGamepadButtonPressed(); });
        rt->add_function("ray_get_gamepad_axis_count", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return GetGamepadAxisCount(SCFXNUMARG(0, int)); });
        rt->add_function("ray_get_gamepad_axis_movement", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) return GetGamepadAxisMovement(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); });
        rt->add_function("ray_set_gamepad_mappings", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return SetGamepadMappings(args[0].cast_to_string().c_str()); });
        rt->add_function("ray_set_gamepad_vibration", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(4) SetGamepadVibration(SCFXNUMARG(0, int), SCFXNUMARG(1, float), SCFXNUMARG(2, float), SCFXNUMARG(3, float)); return {}; });

        rt->add_function("ray_is_mouse_button_pressed", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsMouseButtonPressed(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_mouse_button_down", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsMouseButtonDown(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_mouse_button_released", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsMouseButtonReleased(SCFXNUMARG(0, int)); });
        rt->add_function("ray_is_mouse_button_up", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return IsMouseButtonUp(SCFXNUMARG(0, int)); });
        rt->add_function("ray_get_mouse_x", SCFXFUN(/*fname*/, /*args*/) { return GetMouseX(); });
        rt->add_function("ray_get_mouse_y", SCFXFUN(/*fname*/, /*args*/) { return GetMouseY(); });
        rt->add_function("ray_get_mouse_position", SCFXFUN(/*fname*/, /*args*/) { return scfx::valbox{GetMousePosition(), "Vector2"}; });
        rt->add_function("ray_get_mouse_delta", SCFXFUN(/*fname*/, /*args*/) { return scfx::valbox{GetMouseDelta(), "Vector2"}; });
        rt->add_function("ray_set_mouse_position", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SetMousePosition(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); return {}; });
        rt->add_function("ray_set_mouse_offset", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SetMouseOffset(SCFXNUMARG(0, int), SCFXNUMARG(1, int)); return {}; });
        rt->add_function("ray_set_mouse_scale", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(2) SetMouseScale(SCFXNUMARG(0, float), SCFXNUMARG(1, float)); return {}; });
        rt->add_function("ray_get_mouse_wheel_move", SCFXFUN(/*fname*/, /*args*/) { return GetMouseWheelMove(); });
        rt->add_function("ray_get_mouse_wheel_move_v", SCFXFUN(/*fname*/, /*args*/) { return scfx::valbox{GetMouseWheelMoveV(), "Vector2"}; });
        rt->add_function("ray_set_mouse_cursor", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) SetMouseCursor(SCFXNUMARG(0, int)); return {}; });
        rt->add_function("ray_get_touch_x", SCFXFUN(/*fname*/, /*args*/) { return GetTouchX(); });
        rt->add_function("ray_get_touch_y", SCFXFUN(/*fname*/, /*args*/) { return GetTouchY(); });
        rt->add_function("ray_get_touch_position", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return scfx::valbox{GetTouchPosition(SCFXNUMARG(0, int)), "Vector2"}; });
        rt->add_function("ray_get_touch_point_id", SCFXFUN(/*fname*/, args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(1) return GetTouchPointId(SCFXNUMARG(0, int)); });
        rt->add_function("ray_get_touch_point_count", SCFXFUN(/*fname*/, /*args*/) { return GetTouchPointCount(); });
    }

    void unregister_runtime() override {
        std::unique_lock l{rt_mtp_};
        if(rt_ == nullptr) {
            return;
        }
        rt_->remove_function("Vector2");
        rt_->remove_method("Vector2", "x");
        rt_->remove_method("Vector2", "y");
        rt_->remove_method("Vector2", "set_x");
        rt_->remove_method("Vector2", "set_y");
        rt_->remove_function("Vector3");
        rt_->remove_method("Vector3", "x");
        rt_->remove_method("Vector3", "y");
        rt_->remove_method("Vector3", "z");
        rt_->remove_method("Vector3", "set_x");
        rt_->remove_method("Vector3", "set_y");
        rt_->remove_method("Vector3", "set_z");

        rt_->remove_function("Vector4");
        rt_->remove_function("Quaternion");
        rt_->remove_method("Vector4", "x");
        rt_->remove_method("Vector4", "y");
        rt_->remove_method("Vector4", "z");
        rt_->remove_method("Vector4", "w");
        rt_->remove_method("Vector4", "set_x");
        rt_->remove_method("Vector4", "set_y");
        rt_->remove_method("Vector4", "set_z");
        rt_->remove_method("Vector4", "set_w");

        rt_->remove_function("Color");
        rt_->remove_method("Color", "r");
        rt_->remove_method("Color", "g");
        rt_->remove_method("Color", "b");
        rt_->remove_method("Color", "a");
        rt_->remove_method("Color", "set_r");
        rt_->remove_method("Color", "set_g");
        rt_->remove_method("Color", "set_b");
        rt_->remove_method("Color", "set_a");

        rt_->remove_var("LIGHTGRAY");
        rt_->remove_var("GRAY");
        rt_->remove_var("DARKGRAY");
        rt_->remove_var("YELLOW");
        rt_->remove_var("GOLD");
        rt_->remove_var("ORANGE");
        rt_->remove_var("PINK");
        rt_->remove_var("RED");
        rt_->remove_var("MAROON");
        rt_->remove_var("GREEN");
        rt_->remove_var("LIME");
        rt_->remove_var("DARKGREEN");
        rt_->remove_var("SKYBLUE");
        rt_->remove_var("BLUE");
        rt_->remove_var("DARKBLUE");
        rt_->remove_var("PURPLE");
        rt_->remove_var("VIOLET");
        rt_->remove_var("DARKPURPLE");
        rt_->remove_var("BEIGE");
        rt_->remove_var("BROWN");
        rt_->remove_var("DARKBROWN");
        rt_->remove_var("WHITE");
        rt_->remove_var("BLACK");
        rt_->remove_var("BLANK");
        rt_->remove_var("MAGENTA");
        rt_->remove_var("RAYWHITE");

        rt_->remove_function("ray_get_screen_width");
        rt_->remove_function("ray_get_screen_height");
        rt_->remove_function("ray_get_monitor_count");
        rt_->remove_function("ray_get_current_monitor");
        rt_->remove_function("ray_get_monitor_width");
        rt_->remove_function("ray_get_monitor_height");

        rt_->remove_function("ray_init_window");
        rt_->remove_function("ray_window_should_close");
        rt_->remove_function("ray_begin_drawing");
        rt_->remove_function("ray_end_drawing");
        rt_->remove_function("ray_close_window");
        rt_->remove_function("ray_clear_background");

        rt_->remove_function("ray_set_target_fps");
        rt_->remove_function("ray_toggle_full_screen");
        rt_->remove_function("ray_draw_pixel");
        rt_->remove_function("ray_draw_pixel_v");
        rt_->remove_function("ray_draw_line");
        rt_->remove_function("ray_draw_line_v");
        rt_->remove_function("ray_draw_line_ex");
        rt_->remove_function("ray_draw_line_strip");
        rt_->remove_function("ray_draw_line_bezier");
        rt_->remove_function("ray_draw_circle");
        rt_->remove_function("ray_draw_text");
        rt_->remove_function("ray_draw_rectangle");

        rt_->remove_function("ray_is_key_pressed");
        rt_->remove_function("ray_is_key_pressed_repeat");
        rt_->remove_function("ray_is_key_down");
        rt_->remove_function("ray_is_key_released");
        rt_->remove_function("ray_is_key_up");
        rt_->remove_function("ray_get_key_pressed");
        rt_->remove_function("ray_get_char_pressed");
        rt_->remove_function("ray_set_exit_key");
        rt_->remove_function("ray_is_gamepad_available");
        rt_->remove_function("ray_get_gamepad_name");
        rt_->remove_function("ray_is_gamepad_button_pressed");
        rt_->remove_function("ray_is_gamepad_button_down");
        rt_->remove_function("ray_is_gamepad_button_released");
        rt_->remove_function("ray_is_gamepad_button_up");
        rt_->remove_function("ray_get_gamepad_button_pressed");
        rt_->remove_function("ray_get_gamepad_axis_count");
        rt_->remove_function("ray_get_gamepad_axis_movement");
        rt_->remove_function("ray_set_gamepad_mappings");
        rt_->remove_function("ray_set_gamepad_vibration");

        rt_->remove_function("ray_is_mouse_button_pressed");
        rt_->remove_function("ray_is_mouse_button_down");
        rt_->remove_function("ray_is_mouse_button_released");
        rt_->remove_function("ray_is_mouse_button_up");
        rt_->remove_function("ray_get_mouse_x");
        rt_->remove_function("ray_get_mouse_y");
        rt_->remove_function("ray_get_mouse_position");
        rt_->remove_function("ray_get_mouse_delta");
        rt_->remove_function("ray_set_mouse_position");
        rt_->remove_function("ray_set_mouse_offset");
        rt_->remove_function("ray_set_mouse_scale");
        rt_->remove_function("ray_get_mouse_wheel_move");
        rt_->remove_function("ray_get_mouse_wheel_move_v");
        rt_->remove_function("ray_set_mouse_cursor");
        rt_->remove_function("ray_get_touch_x");
        rt_->remove_function("ray_get_touch_y");
        rt_->remove_function("ray_get_touch_position");
        rt_->remove_function("ray_get_touch_point_id");
        rt_->remove_function("ray_get_touch_point_count");

        rt_ = nullptr;
    }

private:
    std::shared_mutex rt_mtp_{};
    scfx::runtime_interface *rt_{nullptr};
};
