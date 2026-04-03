#include "../../src/tealscript_interfaces.hpp"

#include <raylib.h>
#include <raymath.h>

class ray_ext: public teal::extension_interface {
    teal::valbox color_val(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
        return teal::valbox{Color{r, g, b, a}, "Color"};
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

    void register_runtime(teal::runtime_interface *rt) override {
        std::unique_lock l{rt_mtp_};
        if(rt_ != nullptr) {
            return;
        }
        rt_ = rt;
        if(rt_ == nullptr) {
            return;
        }
        rt->add_function("Vector2", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            return teal::valbox{
                Vector2{
                    args.size() > 0 ? TEALNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 1, float) : 0
                },
                "Vector2"
            };
        });
        rt->add_method("Vector2", "x", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).x; });
        rt->add_method("Vector2", "y", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).y; });
        rt->add_method("Vector2", "set_x", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).x = TEALNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector2", "set_y", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).y = TEALNUMARG(args, 1, float); return {}; });

        rt->add_function("Vector3", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            return teal::valbox{
                Vector3{
                    args.size() > 0 ? TEALNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 1, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 2, float) : 0
                },
                "Vector3"
            };
        });
        rt->add_method("Vector3", "x", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).x; });
        rt->add_method("Vector3", "y", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).y; });
        rt->add_method("Vector3", "z", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).y; });
        rt->add_method("Vector3", "set_x", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).x = TEALNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector3", "set_y", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).y = TEALNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector3", "set_z", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).y = TEALNUMARG(args, 1, float); return {}; });

        rt->add_function("Vector4", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return teal::valbox{
                Vector4{
                    args.size() > 0 ? TEALNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 1, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 2, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_function("Quaternion", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return teal::valbox{
                Vector4{
                    args.size() > 0 ? TEALNUMARG(args, 0, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 1, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 2, float) : 0,
                    args.size() > 0 ? TEALNUMARG(args, 3, float) : 0
                },
                "Vector4"
            };
        });
        rt->add_method("Vector4", "x", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).x; });
        rt->add_method("Vector4", "y", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).y; });
        rt->add_method("Vector4", "z", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).y; });
        rt->add_method("Vector4", "w", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Vector2).y; });
        rt->add_method("Vector4", "set_x", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).x = TEALNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_y", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).y = TEALNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_z", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).y = TEALNUMARG(args, 1, float); return {}; });
        rt->add_method("Vector4", "set_w", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) TEALTHIS(args, Vector2 &).y = TEALNUMARG(args, 1, float); return {}; });

        rt->add_function("Color", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            return teal::valbox{
                Color{
                    args.size() > 0 ? TEALNUMARG(args, 0, uint8_t) : (uint8_t)0,
                    args.size() > 1 ? TEALNUMARG(args, 1, uint8_t) : (uint8_t)0,
                    args.size() > 2 ? TEALNUMARG(args, 2, uint8_t) : (uint8_t)0,
                    args.size() > 3 ? TEALNUMARG(args, 3, uint8_t) : (uint8_t)255
                },
                "Color"
            };
        });
        rt->add_method("Color", "r", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Color).r; });
        rt->add_method("Color", "g", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Color).g; });
        rt->add_method("Color", "b", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Color).b; });
        rt->add_method("Color", "a", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return TEALTHIS(args, Color).a; });
        rt->add_method("Color", "set_r", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) TEALTHIS(args, Color &).r = TEALNUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_g", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) TEALTHIS(args, Color &).g = TEALNUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_b", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) TEALTHIS(args, Color &).b = TEALNUMARG(args, 1, uint8_t); return {}; });
        rt->add_method("Color", "set_a", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) TEALTHIS(args, Color &).a = TEALNUMARG(args, 1, uint8_t); return {}; });

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


        rt->add_function("ray_get_screen_width", TEALFUN() { return GetScreenWidth(); });
        rt->add_function("ray_get_screen_height", TEALFUN() { return GetScreenHeight(); });
        rt->add_function("ray_get_monitor_count", TEALFUN() { return GetMonitorCount(); });
        rt->add_function("ray_get_current_monitor", TEALFUN() { return GetCurrentMonitor(); });
        rt->add_function("ray_get_monitor_width", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorWidth(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_get_monitor_height", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetMonitorHeight(TEALNUMARG(args, 0, int)); });


        rt->add_function("ray_init_window", TEALFUN(args) {
            InitWindow(
                args.size() > 0 ? TEALNUMARG(args, 0, int) > 0 ? TEALNUMARG(args, 0, int) : 800 : 800,
                args.size() > 1 ? TEALNUMARG(args, 1, int) > 0 ? TEALNUMARG(args, 1, int) : 600 : 600,
                args.size() > 2 ? args[2].cast_to_string().c_str() : "Raylib Window"
            );
            return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });
        rt->add_function("ray_window_should_close", TEALFUN() { return WindowShouldClose(); });
        rt->add_function("ray_begin_drawing", TEALFUN() { BeginDrawing(); return teal::valbox{teal::valbox_no_initialize::dont_do_it}; });
        rt->add_function("ray_end_drawing", TEALFUN() { EndDrawing(); return teal::valbox{teal::valbox_no_initialize::dont_do_it}; });
        rt->add_function("ray_close_window", TEALFUN() { CloseWindow(); return teal::valbox{teal::valbox_no_initialize::dont_do_it}; });
        rt->add_function("ray_clear_background", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            ClearBackground(TEALTHIS(args, Color));
            return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        // SetTargetFPS
        rt->add_function("ray_set_target_fps", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
            SetTargetFPS(args[0].cast_to_s32());
            return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        rt->add_function("ray_toggle_full_screen", TEALFUN() { ToggleFullscreen(); return teal::valbox{teal::valbox_no_initialize::dont_do_it}; });

        rt->add_function("ray_draw_pixel", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawPixel(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int), TEALCLASSARG(args, 2, Color));
            return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });
        rt->add_function("ray_draw_pixel_v", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            DrawPixelV(TEALTHIS(args, Vector2), TEALCLASSARG(args, 1, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        rt->add_function("ray_draw_line", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawLine(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int), TEALNUMARG(args, 2, int), TEALNUMARG(args, 3, int), TEALCLASSARG(args, 4, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });
        rt->add_function("ray_draw_line_v", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 3)
            DrawLineV(TEALTHIS(args, Vector2), TEALCLASSARG(args, 1, Vector2), TEALCLASSARG(args, 2, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });
        rt->add_function("ray_draw_line_ex", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineEx(TEALTHIS(args, Vector2), TEALCLASSARG(args, 1, Vector2), TEALNUMARG(args, 2, float), TEALCLASSARG(args, 3, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        rt->add_function("ray_draw_line_strip", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
            auto &&vec2arr{args[0].as_array()};
            std::vector<Vector2> pts{};
            pts.reserve(vec2arr.size());
            for(auto &&v: vec2arr) {
                pts.push_back(v.as_class<Vector2>());
            }
            DrawLineStrip(pts.data(), pts.size(), TEALCLASSARG(args, 1, Color));
            return {};
        });


        rt->add_function("ray_draw_line_bezier", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawLineBezier(TEALTHIS(args, Vector2), TEALCLASSARG(args, 1, Vector2), TEALNUMARG(args, 2, int), TEALCLASSARG(args, 3, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });
        rt->add_function("ray_draw_circle", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4)
            DrawCircle(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int), TEALNUMARG(args, 2, float), TEALCLASSARG(args, 3, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        rt->add_function("ray_draw_text", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawText(
                args[0].cast_to_string().c_str(),
                TEALNUMARG(args, 1, int), TEALNUMARG(args, 2, int),
                TEALNUMARG(args, 3, int),
                TEALCLASSARG(args, 4, Color)
            );
            return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        rt->add_function("ray_draw_rectangle", TEALFUN(args) {
            TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 5)
            DrawRectangle(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int), TEALNUMARG(args, 2, int), TEALNUMARG(args, 3, int), TEALCLASSARG(args, 4, Color)); return teal::valbox{teal::valbox_no_initialize::dont_do_it};
        });

        rt->add_function("ray_is_key_pressed", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyPressed(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_pressed_repeat", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyPressedRepeat(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_down", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyDown(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_released", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyReleased(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_key_up", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsKeyUp(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_get_key_pressed", TEALFUN() { return GetKeyPressed(); });
        rt->add_function("ray_get_char_pressed", TEALFUN() { return GetCharPressed(); });
        rt->add_function("ray_set_exit_key", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) SetExitKey(TEALNUMARG(args, 0, int)); return {}; });
        rt->add_function("ray_is_gamepad_available", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsGamepadAvailable(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_get_gamepad_name", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return std::string{GetGamepadName(TEALNUMARG(args, 0, int))}; });
        rt->add_function("ray_is_gamepad_button_pressed", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonPressed(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_down", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonDown(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_released", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonReleased(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); });
        rt->add_function("ray_is_gamepad_button_up", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) return IsGamepadButtonUp(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); });
        rt->add_function("ray_get_gamepad_button_pressed", TEALFUN() { return GetGamepadButtonPressed(); });
        rt->add_function("ray_get_gamepad_axis_count", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetGamepadAxisCount(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_get_gamepad_axis_movement", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) return GetGamepadAxisMovement(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); });
        rt->add_function("ray_set_gamepad_mappings", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return SetGamepadMappings(args[0].cast_to_string().c_str()); });
        rt->add_function("ray_set_gamepad_vibration", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 4) SetGamepadVibration(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, float), TEALNUMARG(args, 2, float), TEALNUMARG(args, 3, float)); return {}; });

        rt->add_function("ray_is_mouse_button_pressed", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonPressed(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_down", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonDown(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_released", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonReleased(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_is_mouse_button_up", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return IsMouseButtonUp(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_get_mouse_x", TEALFUN() { return GetMouseX(); });
        rt->add_function("ray_get_mouse_y", TEALFUN() { return GetMouseY(); });
        rt->add_function("ray_get_mouse_position", TEALFUN() { return teal::valbox{GetMousePosition(), "Vector2"}; });
        rt->add_function("ray_get_mouse_delta", TEALFUN() { return teal::valbox{GetMouseDelta(), "Vector2"}; });
        rt->add_function("ray_set_mouse_position", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMousePosition(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); return {}; });
        rt->add_function("ray_set_mouse_offset", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMouseOffset(TEALNUMARG(args, 0, int), TEALNUMARG(args, 1, int)); return {}; });
        rt->add_function("ray_set_mouse_scale", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2) SetMouseScale(TEALNUMARG(args, 0, float), TEALNUMARG(args, 1, float)); return {}; });
        rt->add_function("ray_get_mouse_wheel_move", TEALFUN() { return GetMouseWheelMove(); });
        rt->add_function("ray_get_mouse_wheel_move_v", TEALFUN() { return teal::valbox{GetMouseWheelMoveV(), "Vector2"}; });
        rt->add_function("ray_set_mouse_cursor", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) SetMouseCursor(TEALNUMARG(args, 0, int)); return {}; });
        rt->add_function("ray_get_touch_x", TEALFUN() { return GetTouchX(); });
        rt->add_function("ray_get_touch_y", TEALFUN() { return GetTouchY(); });
        rt->add_function("ray_get_touch_position", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return teal::valbox{GetTouchPosition(TEALNUMARG(args, 0, int)), "Vector2"}; });
        rt->add_function("ray_get_touch_point_id", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return GetTouchPointId(TEALNUMARG(args, 0, int)); });
        rt->add_function("ray_get_touch_point_count", TEALFUN() { return GetTouchPointCount(); });
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
    teal::shared_mutex rt_mtp_{};
    teal::runtime_interface *rt_{nullptr};
};

teal::extension_interface *create_teal_extension() {
    static ray_ext extinst{};
    return &extinst;
}

void remove_teal_extension(teal::extension_interface *) {
}
