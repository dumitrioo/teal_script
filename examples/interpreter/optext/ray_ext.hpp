#pragma once

#include <scaflux_value.hpp>
#include <scaflux_util.hpp>
#include <scaflux_interfaces.hpp>

#include <raylib.h>
#include <raymath.h>

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
        rt->add_function("Vector2", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            return scfx::valbox{
                Vector2{
                    args.size() > 0 ? SCFXNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 1, float) : 0
                },
                "Vector2"
            };
        });
        rt->add_method("Vector2", "x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).x; });
        rt->add_method("Vector2", "y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).y; });
        rt->add_method("Vector2", "set_x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).x = SCFXNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector2", "set_y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).y = SCFXNUMARG(args, 1, float); return {}; });

        rt->add_function("Vector3", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            return scfx::valbox{
                Vector3{
                    args.size() > 0 ? SCFXNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 1, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 2, float) : 0
                },
                "Vector3"
            };
        });
        rt->add_method("Vector3", "x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).x; });
        rt->add_method("Vector3", "y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).y; });
        rt->add_method("Vector3", "z", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).y; });
        rt->add_method("Vector3", "set_x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).x = SCFXNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector3", "set_y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).y = SCFXNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector3", "set_z", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).y = SCFXNUMARG(args, 1, float); return {}; });

        rt->add_function("Vector4", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return scfx::valbox{
                Vector4{
                    args.size() > 0 ? SCFXNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 1, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 2, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_function("Quaternion", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return scfx::valbox{
                Vector4{
                    args.size() > 0 ? SCFXNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 1, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 2, float) : 0,
                    args.size() > 0 ? SCFXNUMARG(args, 3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_method("Vector4", "x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).x; });
        rt->add_method("Vector4", "y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).y; });
        rt->add_method("Vector4", "z", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).y; });
        rt->add_method("Vector4", "w", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Vector2).y; });
        rt->add_method("Vector4", "set_x", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).x = SCFXNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_y", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).y = SCFXNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_z", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).y = SCFXNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_w", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SCFXTHIS(args, Vector2 &).y = SCFXNUMARG(args, 1, float); return {}; });

        rt->add_function("Color", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return scfx::valbox{
                Color{
                    args.size() > 0 ? SCFXNUMARG(args, 0, uint8_t) : (uint8_t)0,
                    args.size() > 1 ? SCFXNUMARG(args, 1, uint8_t) : (uint8_t)0,
                    args.size() > 2 ? SCFXNUMARG(args, 2, uint8_t) : (uint8_t)0,
                    args.size() > 3 ? SCFXNUMARG(args, 3, uint8_t) : (uint8_t)255
                },
                "Color"
            };
        });
        rt->add_method("Color", "r", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Color).r; });
        rt->add_method("Color", "g", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Color).g; });
        rt->add_method("Color", "b", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Color).b; });
        rt->add_method("Color", "a", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SCFXTHIS(args, Color).a; });
        rt->add_method("Color", "set_r", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, Color &).r = SCFXNUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_g", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, Color &).g = SCFXNUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_b", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, Color &).b = SCFXNUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_a", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SCFXTHIS(args, Color &).a = SCFXNUMARG(args, 1, uint8_t); return {}; });

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


        rt->add_function("ray_get_screen_width", SCFXFUN() { return GetScreenWidth(); });
        rt->add_function("ray_get_screen_height", SCFXFUN() { return GetScreenHeight(); });
        rt->add_function("ray_get_monitor_count", SCFXFUN() { return GetMonitorCount(); });
        rt->add_function("ray_get_current_monitor", SCFXFUN() { return GetCurrentMonitor(); });
        rt->add_function("ray_get_monitor_width", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorWidth(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_get_monitor_height", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorHeight(SCFXNUMARG(args, 0, int)); });


        rt->add_function("ray_init_window", SCFXFUN(args) {
            InitWindow(
                args.size() > 0 ? SCFXNUMARG(args, 0, int) > 0 ? SCFXNUMARG(args, 0, int) : 800 : 800,
                args.size() > 1 ? SCFXNUMARG(args, 1, int) > 0 ? SCFXNUMARG(args, 1, int) : 600 : 600,
                args.size() > 2 ? args[2].cast_to_string().c_str() : "Raylib Window"
            );
            return scfx::valbox{};
        });
        rt->add_function("ray_window_should_close", SCFXFUN() { return WindowShouldClose(); });
        rt->add_function("ray_begin_drawing", SCFXFUN() { BeginDrawing(); return scfx::valbox{}; });
        rt->add_function("ray_end_drawing", SCFXFUN() { EndDrawing(); return scfx::valbox{}; });
        rt->add_function("ray_close_window", SCFXFUN() { CloseWindow(); return scfx::valbox{}; });
        rt->add_function("ray_clear_background", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            ClearBackground(SCFXTHIS(args, Color));
            return scfx::valbox{};
        });

        // SetTargetFPS
        rt->add_function("ray_set_target_fps", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            SetTargetFPS(args[0].cast_to_s32());
            return scfx::valbox{};
        });

        rt->add_function("ray_toggle_full_screen", SCFXFUN() { ToggleFullscreen(); return scfx::valbox{}; });

        rt->add_function("ray_draw_pixel", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawPixel(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int), SCFXCLASSARG(args, 2, Color));
            return scfx::valbox{};
        });
        rt->add_function("ray_draw_pixel_v", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            DrawPixelV(SCFXTHIS(args, Vector2), SCFXCLASSARG(args, 1, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_draw_line", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawLine(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int), SCFXNUMARG(args, 2, int), SCFXNUMARG(args, 3, int), SCFXCLASSARG(args, 4, Color)); return scfx::valbox{};
        });
        rt->add_function("ray_draw_line_v", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawLineV(SCFXTHIS(args, Vector2), SCFXCLASSARG(args, 1, Vector2), SCFXCLASSARG(args, 2, Color)); return scfx::valbox{};
        });
        rt->add_function("ray_draw_line_ex", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineEx(SCFXTHIS(args, Vector2), SCFXCLASSARG(args, 1, Vector2), SCFXNUMARG(args, 2, float), SCFXCLASSARG(args, 3, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_draw_line_strip", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            auto &&vec2arr{args[0].as_array()};
            std::vector<Vector2> pts{};
            pts.reserve(vec2arr.size());
            for(auto &&v: vec2arr) {
                pts.push_back(v.as_class<Vector2>());
            }
            DrawLineStrip(pts.data(), pts.size(), SCFXCLASSARG(args, 1, Color));
            return {};
        });


        rt->add_function("ray_draw_line_bezier", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineBezier(SCFXTHIS(args, Vector2), SCFXCLASSARG(args, 1, Vector2), SCFXNUMARG(args, 2, int), SCFXCLASSARG(args, 3, Color)); return scfx::valbox{};
        });
        rt->add_function("ray_draw_circle", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawCircle(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int), SCFXNUMARG(args, 2, float), SCFXCLASSARG(args, 3, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_draw_text", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawText(
                args[0].cast_to_string().c_str(),
                SCFXNUMARG(args, 1, int), SCFXNUMARG(args, 2, int),
                SCFXNUMARG(args, 3, int),
                SCFXCLASSARG(args, 4, Color)
            );
            return scfx::valbox{};
        });

        rt->add_function("ray_draw_rectangle", SCFXFUN(args) {
            SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawRectangle(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int), SCFXNUMARG(args, 2, int), SCFXNUMARG(args, 3, int), SCFXCLASSARG(args, 4, Color)); return scfx::valbox{};
        });

        rt->add_function("ray_is_key_pressed", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyPressed(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_pressed_repeat", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyPressedRepeat(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_down", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyDown(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_released", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyReleased(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_up", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyUp(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_get_key_pressed", SCFXFUN() { return GetKeyPressed(); });
        rt->add_function("ray_get_char_pressed", SCFXFUN() { return GetCharPressed(); });
        rt->add_function("ray_set_exit_key", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SetExitKey(SCFXNUMARG(args, 0, int)); return {}; });
        rt->add_function("ray_is_gamepad_available", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsGamepadAvailable(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_get_gamepad_name", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return std::string{GetGamepadName(SCFXNUMARG(args, 0, int))}; });
        rt->add_function("ray_is_gamepad_button_pressed", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonPressed(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_down", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonDown(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_released", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonReleased(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_up", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonUp(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); });
        rt->add_function("ray_get_gamepad_button_pressed", SCFXFUN() { return GetGamepadButtonPressed(); });
        rt->add_function("ray_get_gamepad_axis_count", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetGamepadAxisCount(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_get_gamepad_axis_movement", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) return GetGamepadAxisMovement(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); });
        rt->add_function("ray_set_gamepad_mappings", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SetGamepadMappings(args[0].cast_to_string().c_str()); });
        rt->add_function("ray_set_gamepad_vibration", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 4) SetGamepadVibration(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, float), SCFXNUMARG(args, 2, float), SCFXNUMARG(args, 3, float)); return {}; });

        rt->add_function("ray_is_mouse_button_pressed", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonPressed(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_down", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonDown(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_released", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonReleased(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_up", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonUp(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_get_mouse_x", SCFXFUN() { return GetMouseX(); });
        rt->add_function("ray_get_mouse_y", SCFXFUN() { return GetMouseY(); });
        rt->add_function("ray_get_mouse_position", SCFXFUN() { return scfx::valbox{GetMousePosition(), "Vector2"}; });
        rt->add_function("ray_get_mouse_delta", SCFXFUN() { return scfx::valbox{GetMouseDelta(), "Vector2"}; });
        rt->add_function("ray_set_mouse_position", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMousePosition(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); return {}; });
        rt->add_function("ray_set_mouse_offset", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMouseOffset(SCFXNUMARG(args, 0, int), SCFXNUMARG(args, 1, int)); return {}; });
        rt->add_function("ray_set_mouse_scale", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMouseScale(SCFXNUMARG(args, 0, float), SCFXNUMARG(args, 1, float)); return {}; });
        rt->add_function("ray_get_mouse_wheel_move", SCFXFUN() { return GetMouseWheelMove(); });
        rt->add_function("ray_get_mouse_wheel_move_v", SCFXFUN() { return scfx::valbox{GetMouseWheelMoveV(), "Vector2"}; });
        rt->add_function("ray_set_mouse_cursor", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) SetMouseCursor(SCFXNUMARG(args, 0, int)); return {}; });
        rt->add_function("ray_get_touch_x", SCFXFUN() { return GetTouchX(); });
        rt->add_function("ray_get_touch_y", SCFXFUN() { return GetTouchY(); });
        rt->add_function("ray_get_touch_position", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return scfx::valbox{GetTouchPosition(SCFXNUMARG(args, 0, int)), "Vector2"}; });
        rt->add_function("ray_get_touch_point_id", SCFXFUN(args) { SCFX_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetTouchPointId(SCFXNUMARG(args, 0, int)); });
        rt->add_function("ray_get_touch_point_count", SCFXFUN() { return GetTouchPointCount(); });
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
    scfx::shared_mutex rt_mtp_{};
    scfx::runtime_interface *rt_{nullptr};
};
