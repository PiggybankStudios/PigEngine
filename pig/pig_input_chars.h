/*
File:   pig_input_chars.h
Author: Taylor Robbins
Date:   01\21\2022
Description:
	** We want to be able to easily draw input glyphs in our test information.
	** Since I can't find a place in unicode that has pre-defined characters for
	** keyboard and gamepad input like glyphs, we took over the "Chess Checkers"
	** Miscellaneous Symbols page that starts at 0x2600 and goes to 0x26FF
*/

#ifndef _PIG_INPUT_CHARS_H
#define _PIG_INPUT_CHARS_H

#define INPUT_DISP_CHAR_ESCAPE_CODEPOINT             0x00002600
#define INPUT_DISP_CHAR_F1_CODEPOINT                 0x00002601
#define INPUT_DISP_CHAR_F2_CODEPOINT                 0x00002602
#define INPUT_DISP_CHAR_F3_CODEPOINT                 0x00002603
#define INPUT_DISP_CHAR_F4_CODEPOINT                 0x00002604
#define INPUT_DISP_CHAR_F5_CODEPOINT                 0x00002605
#define INPUT_DISP_CHAR_F6_CODEPOINT                 0x00002606
#define INPUT_DISP_CHAR_F7_CODEPOINT                 0x00002607
#define INPUT_DISP_CHAR_F8_CODEPOINT                 0x00002608
#define INPUT_DISP_CHAR_F9_CODEPOINT                 0x00002609
#define INPUT_DISP_CHAR_F10_CODEPOINT                0x0000260A
#define INPUT_DISP_CHAR_F11_CODEPOINT                0x0000260B
#define INPUT_DISP_CHAR_F12_CODEPOINT                0x0000260C
#define INPUT_DISP_CHAR_SPACE_CODEPOINT              0x0000260D
#define INPUT_DISP_CHAR_BLANK_CODEPOINT              0x0000260E
#define INPUT_DISP_CHAR_BLANK_WIDE_CODEPOINT         0x0000260F
#define INPUT_DISP_CHAR_TILDE_CODEPOINT              0x00002610
#define INPUT_DISP_CHAR_1_CODEPOINT                  0x00002611
#define INPUT_DISP_CHAR_2_CODEPOINT                  0x00002612
#define INPUT_DISP_CHAR_3_CODEPOINT                  0x00002613
#define INPUT_DISP_CHAR_4_CODEPOINT                  0x00002614
#define INPUT_DISP_CHAR_5_CODEPOINT                  0x00002615
#define INPUT_DISP_CHAR_6_CODEPOINT                  0x00002616
#define INPUT_DISP_CHAR_7_CODEPOINT                  0x00002617
#define INPUT_DISP_CHAR_8_CODEPOINT                  0x00002618
#define INPUT_DISP_CHAR_9_CODEPOINT                  0x00002619
#define INPUT_DISP_CHAR_0_CODEPOINT                  0x0000261A
#define INPUT_DISP_CHAR_MINUS_CODEPOINT              0x0000261B
#define INPUT_DISP_CHAR_PLUS_CODEPOINT               0x0000261C
#define INPUT_DISP_CHAR_BACK_CODEPOINT               0x0000261D
#define INPUT_DISP_CHAR_INSERT_CODEPOINT             0x0000261E
#define INPUT_DISP_CHAR_DELETE_CODEPOINT             0x0000261F
#define INPUT_DISP_CHAR_TAB_CODEPOINT                0x00002620
#define INPUT_DISP_CHAR_Q_CODEPOINT                  0x00002621
#define INPUT_DISP_CHAR_W_CODEPOINT                  0x00002622
#define INPUT_DISP_CHAR_E_CODEPOINT                  0x00002623
#define INPUT_DISP_CHAR_R_CODEPOINT                  0x00002624
#define INPUT_DISP_CHAR_T_CODEPOINT                  0x00002625
#define INPUT_DISP_CHAR_Y_CODEPOINT                  0x00002626
#define INPUT_DISP_CHAR_U_CODEPOINT                  0x00002627
#define INPUT_DISP_CHAR_I_CODEPOINT                  0x00002628
#define INPUT_DISP_CHAR_O_CODEPOINT                  0x00002629
#define INPUT_DISP_CHAR_P_CODEPOINT                  0x0000262A
#define INPUT_DISP_CHAR_OPEN_BRACKET_CODEPOINT       0x0000262B
#define INPUT_DISP_CHAR_CLOSE_BRACKET_CODEPOINT      0x0000262C
#define INPUT_DISP_CHAR_PIPE_CODEPOINT               0x0000262D
#define INPUT_DISP_CHAR_HOME_CODEPOINT               0x0000262E
#define INPUT_DISP_CHAR_END_CODEPOINT                0x0000262F
#define INPUT_DISP_CHAR_CAPS_CODEPOINT               0x00002630
#define INPUT_DISP_CHAR_A_CODEPOINT                  0x00002631
#define INPUT_DISP_CHAR_S_CODEPOINT                  0x00002632
#define INPUT_DISP_CHAR_D_CODEPOINT                  0x00002633
#define INPUT_DISP_CHAR_F_CODEPOINT                  0x00002634
#define INPUT_DISP_CHAR_G_CODEPOINT                  0x00002635
#define INPUT_DISP_CHAR_H_CODEPOINT                  0x00002636
#define INPUT_DISP_CHAR_J_CODEPOINT                  0x00002637
#define INPUT_DISP_CHAR_K_CODEPOINT                  0x00002638
#define INPUT_DISP_CHAR_L_CODEPOINT                  0x00002639
#define INPUT_DISP_CHAR_COLON_CODEPOINT              0x0000263A
#define INPUT_DISP_CHAR_QUOTE_CODEPOINT              0x0000263B
#define INPUT_DISP_CHAR_UP_CODEPOINT                 0x0000263C
#define INPUT_DISP_CHAR_ENTER_CODEPOINT              0x0000263D
#define INPUT_DISP_CHAR_PAGE_UP_CODEPOINT            0x0000263E
#define INPUT_DISP_CHAR_PAGE_DOWN_CODEPOINT          0x0000263F
#define INPUT_DISP_CHAR_SHIFT_CODEPOINT              0x00002640
#define INPUT_DISP_CHAR_Z_CODEPOINT                  0x00002641
#define INPUT_DISP_CHAR_X_CODEPOINT                  0x00002642
#define INPUT_DISP_CHAR_C_CODEPOINT                  0x00002643
#define INPUT_DISP_CHAR_V_CODEPOINT                  0x00002644
#define INPUT_DISP_CHAR_B_CODEPOINT                  0x00002645
#define INPUT_DISP_CHAR_N_CODEPOINT                  0x00002646
#define INPUT_DISP_CHAR_M_CODEPOINT                  0x00002647
#define INPUT_DISP_CHAR_COMMA_CODEPOINT              0x00002648
#define INPUT_DISP_CHAR_PERIOD_CODEPOINT             0x00002649
#define INPUT_DISP_CHAR_QUESTION_CODEPOINT           0x0000264A
#define INPUT_DISP_CHAR_LEFT_CODEPOINT               0x0000264B
#define INPUT_DISP_CHAR_DOWN_CODEPOINT               0x0000264C
#define INPUT_DISP_CHAR_RIGHT_CODEPOINT              0x0000264D
#define INPUT_DISP_CHAR_ALT_CODEPOINT                0x0000264E
#define INPUT_DISP_CHAR_CONTROL_CODEPOINT            0x0000264F
#define INPUT_DISP_CHAR_BLANK_SQUARE_CODEPOINT       0x00002650
#define INPUT_DISP_CHAR_BLANK_SQUARE_LARGE_CODEPOINT 0x00002651
#define INPUT_DISP_CHAR_BLANK_SEMI_WIDE_CODEPOINT    0x00002652
#define INPUT_DISP_CHAR_MOUSE_BLANK_CODEPOINT        0x00002653
#define INPUT_DISP_CHAR_MOUSE_LEFT_CODEPOINT         0x00002654
#define INPUT_DISP_CHAR_MOUSE_RIGHT_CODEPOINT        0x00002655
#define INPUT_DISP_CHAR_MOUSE_MIDDLE_CODEPOINT       0x00002656
#define INPUT_DISP_CHAR_XB_A_CODEPOINT               0x00002660
#define INPUT_DISP_CHAR_XB_B_CODEPOINT               0x00002661
#define INPUT_DISP_CHAR_XB_X_CODEPOINT               0x00002662
#define INPUT_DISP_CHAR_XB_Y_CODEPOINT               0x00002663
#define INPUT_DISP_CHAR_XB_LB_CODEPOINT              0x00002664
#define INPUT_DISP_CHAR_XB_RB_CODEPOINT              0x00002665
#define INPUT_DISP_CHAR_XB_LT_CODEPOINT              0x00002666
#define INPUT_DISP_CHAR_XB_RT_CODEPOINT              0x00002667
#define INPUT_DISP_CHAR_XB_L3_CODEPOINT              0x00002668
#define INPUT_DISP_CHAR_XB_R3_CODEPOINT              0x00002669
#define INPUT_DISP_CHAR_XB_BACK_CODEPOINT            0x0000266A
#define INPUT_DISP_CHAR_XB_START_CODEPOINT           0x0000266B
#define INPUT_DISP_CHAR_XB_UP_CODEPOINT              0x0000266C
#define INPUT_DISP_CHAR_XB_LEFT_CODEPOINT            0x0000266D
#define INPUT_DISP_CHAR_XB_DOWN_CODEPOINT            0x0000266E
#define INPUT_DISP_CHAR_XB_RIGHT_CODEPOINT           0x0000266F
#define INPUT_DISP_CHAR_XB_LEFT_STICK_CODEPOINT      0x00002670
#define INPUT_DISP_CHAR_XB_RIGHT_STICK_CODEPOINT     0x00002671
#define INPUT_DISP_CHAR_XB_DPAD_CODEPOINT            0x00002672
#define INPUT_DISP_CHAR_XB_UNKNOWN_CODEPOINT         0x00002673
#define INPUT_DISP_CHAR_PS_CROSS_CODEPOINT           0x00002680
#define INPUT_DISP_CHAR_PS_CIRCLE_CODEPOINT          0x00002681
#define INPUT_DISP_CHAR_PS_SQUARE_CODEPOINT          0x00002682
#define INPUT_DISP_CHAR_PS_TRIANGLE_CODEPOINT        0x00002683
#define INPUT_DISP_CHAR_PS_L1_CODEPOINT              0x00002684
#define INPUT_DISP_CHAR_PS_R1_CODEPOINT              0x00002685
#define INPUT_DISP_CHAR_PS_L2_CODEPOINT              0x00002686
#define INPUT_DISP_CHAR_PS_R2_CODEPOINT              0x00002687
#define INPUT_DISP_CHAR_PS_L3_CODEPOINT              0x00002688
#define INPUT_DISP_CHAR_PS_R3_CODEPOINT              0x00002689
#define INPUT_DISP_CHAR_PS_SHARE_CODEPOINT           0x0000268A
#define INPUT_DISP_CHAR_PS_OPTIONS_CODEPOINT         0x0000268B
#define INPUT_DISP_CHAR_PS_UP_CODEPOINT              0x0000268C
#define INPUT_DISP_CHAR_PS_LEFT_CODEPOINT            0x0000268D
#define INPUT_DISP_CHAR_PS_DOWN_CODEPOINT            0x0000268E
#define INPUT_DISP_CHAR_PS_RIGHT_CODEPOINT           0x0000268F
#define INPUT_DISP_CHAR_PS_LEFT_STICK_CODEPOINT      0x00002690
#define INPUT_DISP_CHAR_PS_RIGHT_STICK_CODEPOINT     0x00002691
#define INPUT_DISP_CHAR_PS_DPAD_CODEPOINT            0x00002692
#define INPUT_DISP_CHAR_PS_UNKNOWN_CODEPOINT         0x00002693
#define INPUT_DISP_CHAR_NT_B_CODEPOINT               0x000026A0
#define INPUT_DISP_CHAR_NT_A_CODEPOINT               0x000026A1
#define INPUT_DISP_CHAR_NT_Y_CODEPOINT               0x000026A2
#define INPUT_DISP_CHAR_NT_X_CODEPOINT               0x000026A3
#define INPUT_DISP_CHAR_NT_L_CODEPOINT               0x000026A4
#define INPUT_DISP_CHAR_NT_R_CODEPOINT               0x000026A5
#define INPUT_DISP_CHAR_NT_ZL_CODEPOINT              0x000026A6
#define INPUT_DISP_CHAR_NT_ZR_CODEPOINT              0x000026A7
// #define INPUT_DISP_CHAR_NT_L3_CODEPOINT              0x000026A8
// #define INPUT_DISP_CHAR_NT_R3_CODEPOINT              0x000026A9
#define INPUT_DISP_CHAR_NT_MINUS_CODEPOINT           0x000026AA
#define INPUT_DISP_CHAR_NT_PLUS_CODEPOINT            0x000026AB
#define INPUT_DISP_CHAR_NT_UP_CODEPOINT              0x000026AC
#define INPUT_DISP_CHAR_NT_LEFT_CODEPOINT            0x000026AD
#define INPUT_DISP_CHAR_NT_DOWN_CODEPOINT            0x000026AE
#define INPUT_DISP_CHAR_NT_RIGHT_CODEPOINT           0x000026AF
#define INPUT_DISP_CHAR_NT_LEFT_STICK_CODEPOINT      0x000026B0
#define INPUT_DISP_CHAR_NT_RIGHT_STICK_CODEPOINT     0x000026B1
#define INPUT_DISP_CHAR_NT_DPAD_CODEPOINT            0x000026B2
#define INPUT_DISP_CHAR_NT_UNKNOWN_CODEPOINT         0x000026B3

#define INPUT_DISP_CHAR_ESCAPE_STR             u8"\u2600"
#define INPUT_DISP_CHAR_F1_STR                 u8"\u2601"
#define INPUT_DISP_CHAR_F2_STR                 u8"\u2602"
#define INPUT_DISP_CHAR_F3_STR                 u8"\u2603"
#define INPUT_DISP_CHAR_F4_STR                 u8"\u2604"
#define INPUT_DISP_CHAR_F5_STR                 u8"\u2605"
#define INPUT_DISP_CHAR_F6_STR                 u8"\u2606"
#define INPUT_DISP_CHAR_F7_STR                 u8"\u2607"
#define INPUT_DISP_CHAR_F8_STR                 u8"\u2608"
#define INPUT_DISP_CHAR_F9_STR                 u8"\u2609"
#define INPUT_DISP_CHAR_F10_STR                u8"\u260A"
#define INPUT_DISP_CHAR_F11_STR                u8"\u260B"
#define INPUT_DISP_CHAR_F12_STR                u8"\u260C"
#define INPUT_DISP_CHAR_SPACE_STR              u8"\u260D"
#define INPUT_DISP_CHAR_BLANK_STR              u8"\u260E"
#define INPUT_DISP_CHAR_BLANK_WIDE_STR         u8"\u260F"
#define INPUT_DISP_CHAR_TILDE_STR              u8"\u2610"
#define INPUT_DISP_CHAR_1_STR                  u8"\u2611"
#define INPUT_DISP_CHAR_2_STR                  u8"\u2612"
#define INPUT_DISP_CHAR_3_STR                  u8"\u2613"
#define INPUT_DISP_CHAR_4_STR                  u8"\u2614"
#define INPUT_DISP_CHAR_5_STR                  u8"\u2615"
#define INPUT_DISP_CHAR_6_STR                  u8"\u2616"
#define INPUT_DISP_CHAR_7_STR                  u8"\u2617"
#define INPUT_DISP_CHAR_8_STR                  u8"\u2618"
#define INPUT_DISP_CHAR_9_STR                  u8"\u2619"
#define INPUT_DISP_CHAR_0_STR                  u8"\u261A"
#define INPUT_DISP_CHAR_MINUS_STR              u8"\u261B"
#define INPUT_DISP_CHAR_PLUS_STR               u8"\u261C"
#define INPUT_DISP_CHAR_BACK_STR               u8"\u261D"
#define INPUT_DISP_CHAR_INSERT_STR             u8"\u261E"
#define INPUT_DISP_CHAR_DELETE_STR             u8"\u261F"
#define INPUT_DISP_CHAR_TAB_STR                u8"\u2620"
#define INPUT_DISP_CHAR_Q_STR                  u8"\u2621"
#define INPUT_DISP_CHAR_W_STR                  u8"\u2622"
#define INPUT_DISP_CHAR_E_STR                  u8"\u2623"
#define INPUT_DISP_CHAR_R_STR                  u8"\u2624"
#define INPUT_DISP_CHAR_T_STR                  u8"\u2625"
#define INPUT_DISP_CHAR_Y_STR                  u8"\u2626"
#define INPUT_DISP_CHAR_U_STR                  u8"\u2627"
#define INPUT_DISP_CHAR_I_STR                  u8"\u2628"
#define INPUT_DISP_CHAR_O_STR                  u8"\u2629"
#define INPUT_DISP_CHAR_P_STR                  u8"\u262A"
#define INPUT_DISP_CHAR_OPEN_BRACKET_STR       u8"\u262B"
#define INPUT_DISP_CHAR_CLOSE_BRACKET_STR      u8"\u262C"
#define INPUT_DISP_CHAR_PIPE_STR               u8"\u262D"
#define INPUT_DISP_CHAR_HOME_STR               u8"\u262E"
#define INPUT_DISP_CHAR_END_STR                u8"\u262F"
#define INPUT_DISP_CHAR_CAPS_STR               u8"\u2630"
#define INPUT_DISP_CHAR_A_STR                  u8"\u2631"
#define INPUT_DISP_CHAR_S_STR                  u8"\u2632"
#define INPUT_DISP_CHAR_D_STR                  u8"\u2633"
#define INPUT_DISP_CHAR_F_STR                  u8"\u2634"
#define INPUT_DISP_CHAR_G_STR                  u8"\u2635"
#define INPUT_DISP_CHAR_H_STR                  u8"\u2636"
#define INPUT_DISP_CHAR_J_STR                  u8"\u2637"
#define INPUT_DISP_CHAR_K_STR                  u8"\u2638"
#define INPUT_DISP_CHAR_L_STR                  u8"\u2639"
#define INPUT_DISP_CHAR_COLON_STR              u8"\u263A"
#define INPUT_DISP_CHAR_QUOTE_STR              u8"\u263B"
#define INPUT_DISP_CHAR_UP_STR                 u8"\u263C"
#define INPUT_DISP_CHAR_ENTER_STR              u8"\u263D"
#define INPUT_DISP_CHAR_PAGE_UP_STR            u8"\u263E"
#define INPUT_DISP_CHAR_PAGE_DOWN_STR          u8"\u263F"
#define INPUT_DISP_CHAR_SHIFT_STR              u8"\u2640"
#define INPUT_DISP_CHAR_Z_STR                  u8"\u2641"
#define INPUT_DISP_CHAR_X_STR                  u8"\u2642"
#define INPUT_DISP_CHAR_C_STR                  u8"\u2643"
#define INPUT_DISP_CHAR_V_STR                  u8"\u2644"
#define INPUT_DISP_CHAR_B_STR                  u8"\u2645"
#define INPUT_DISP_CHAR_N_STR                  u8"\u2646"
#define INPUT_DISP_CHAR_M_STR                  u8"\u2647"
#define INPUT_DISP_CHAR_COMMA_STR              u8"\u2648"
#define INPUT_DISP_CHAR_PERIOD_STR             u8"\u2649"
#define INPUT_DISP_CHAR_QUESTION_STR           u8"\u264A"
#define INPUT_DISP_CHAR_LEFT_STR               u8"\u264B"
#define INPUT_DISP_CHAR_DOWN_STR               u8"\u264C"
#define INPUT_DISP_CHAR_RIGHT_STR              u8"\u264D"
#define INPUT_DISP_CHAR_ALT_STR                u8"\u264E"
#define INPUT_DISP_CHAR_CONTROL_STR            u8"\u264F"
#define INPUT_DISP_CHAR_BLANK_SQUARE_STR       u8"\u2650"
#define INPUT_DISP_CHAR_BLANK_SQUARE_LARGE_STR u8"\u2651"
#define INPUT_DISP_CHAR_BLANK_SEMI_WIDE_STR    u8"\u2652"
#define INPUT_DISP_CHAR_MOUSE_BLANK_STR        u8"\u2653"
#define INPUT_DISP_CHAR_MOUSE_LEFT_STR         u8"\u2654"
#define INPUT_DISP_CHAR_MOUSE_RIGHT_STR        u8"\u2655"
#define INPUT_DISP_CHAR_MOUSE_MIDDLE_STR       u8"\u2656"
#define INPUT_DISP_CHAR_XB_A_STR               u8"\u2660"
#define INPUT_DISP_CHAR_XB_B_STR               u8"\u2661"
#define INPUT_DISP_CHAR_XB_X_STR               u8"\u2662"
#define INPUT_DISP_CHAR_XB_Y_STR               u8"\u2663"
#define INPUT_DISP_CHAR_XB_LB_STR              u8"\u2664"
#define INPUT_DISP_CHAR_XB_RB_STR              u8"\u2665"
#define INPUT_DISP_CHAR_XB_LT_STR              u8"\u2666"
#define INPUT_DISP_CHAR_XB_RT_STR              u8"\u2667"
#define INPUT_DISP_CHAR_XB_L3_STR              u8"\u2668"
#define INPUT_DISP_CHAR_XB_R3_STR              u8"\u2669"
#define INPUT_DISP_CHAR_XB_BACK_STR            u8"\u266A"
#define INPUT_DISP_CHAR_XB_START_STR           u8"\u266B"
#define INPUT_DISP_CHAR_XB_UP_STR              u8"\u266C"
#define INPUT_DISP_CHAR_XB_LEFT_STR            u8"\u266D"
#define INPUT_DISP_CHAR_XB_DOWN_STR            u8"\u266E"
#define INPUT_DISP_CHAR_XB_RIGHT_STR           u8"\u266F"
#define INPUT_DISP_CHAR_XB_LEFT_STICK_STR      u8"\u2670"
#define INPUT_DISP_CHAR_XB_RIGHT_STICK_STR     u8"\u2671"
#define INPUT_DISP_CHAR_XB_DPAD_STR            u8"\u2672"
#define INPUT_DISP_CHAR_XB_UNKNOWN_STR         u8"\u2673"
#define INPUT_DISP_CHAR_PS_CROSS_STR           u8"\u2680"
#define INPUT_DISP_CHAR_PS_CIRCLE_STR          u8"\u2681"
#define INPUT_DISP_CHAR_PS_SQUARE_STR          u8"\u2682"
#define INPUT_DISP_CHAR_PS_TRIANGLE_STR        u8"\u2683"
#define INPUT_DISP_CHAR_PS_L1_STR              u8"\u2684"
#define INPUT_DISP_CHAR_PS_R1_STR              u8"\u2685"
#define INPUT_DISP_CHAR_PS_L2_STR              u8"\u2686"
#define INPUT_DISP_CHAR_PS_R2_STR              u8"\u2687"
#define INPUT_DISP_CHAR_PS_L3_STR              u8"\u2688"
#define INPUT_DISP_CHAR_PS_R3_STR              u8"\u2689"
#define INPUT_DISP_CHAR_PS_SHARE_STR           u8"\u268A"
#define INPUT_DISP_CHAR_PS_OPTIONS_STR         u8"\u268B"
#define INPUT_DISP_CHAR_PS_UP_STR              u8"\u268C"
#define INPUT_DISP_CHAR_PS_LEFT_STR            u8"\u268D"
#define INPUT_DISP_CHAR_PS_DOWN_STR            u8"\u268E"
#define INPUT_DISP_CHAR_PS_RIGHT_STR           u8"\u268F"
#define INPUT_DISP_CHAR_PS_LEFT_STICK_STR      u8"\u2690"
#define INPUT_DISP_CHAR_PS_RIGHT_STICK_STR     u8"\u2691"
#define INPUT_DISP_CHAR_PS_DPAD_STR            u8"\u2692"
#define INPUT_DISP_CHAR_PS_UNKNOWN_STR         u8"\u2693"
#define INPUT_DISP_CHAR_NT_B_STR               u8"\u26A0"
#define INPUT_DISP_CHAR_NT_A_STR               u8"\u26A1"
#define INPUT_DISP_CHAR_NT_Y_STR               u8"\u26A2"
#define INPUT_DISP_CHAR_NT_X_STR               u8"\u26A3"
#define INPUT_DISP_CHAR_NT_L_STR               u8"\u26A4"
#define INPUT_DISP_CHAR_NT_R_STR               u8"\u26A5"
#define INPUT_DISP_CHAR_NT_ZL_STR              u8"\u26A6"
#define INPUT_DISP_CHAR_NT_ZR_STR              u8"\u26A7"
// #define INPUT_DISP_CHAR_NT_L3_STR              u8"\u26A8"
// #define INPUT_DISP_CHAR_NT_R3_STR              u8"\u26A9"
#define INPUT_DISP_CHAR_NT_MINUS_STR           u8"\u26AA"
#define INPUT_DISP_CHAR_NT_PLUS_STR            u8"\u26AB"
#define INPUT_DISP_CHAR_NT_UP_STR              u8"\u26AC"
#define INPUT_DISP_CHAR_NT_LEFT_STR            u8"\u26AD"
#define INPUT_DISP_CHAR_NT_DOWN_STR            u8"\u26AE"
#define INPUT_DISP_CHAR_NT_RIGHT_STR           u8"\u26AF"
#define INPUT_DISP_CHAR_NT_LEFT_STICK_STR      u8"\u26B0"
#define INPUT_DISP_CHAR_NT_RIGHT_STICK_STR     u8"\u26B1"
#define INPUT_DISP_CHAR_NT_DPAD_STR            u8"\u26B2"
#define INPUT_DISP_CHAR_NT_UNKNOWN_STR         u8"\u26B3"

const char* GetDisplayCharStrForKey(Key_t key)
{
	switch (key)
	{
		case Key_Escape:       return INPUT_DISP_CHAR_ESCAPE_STR;
		case Key_F1:           return INPUT_DISP_CHAR_F1_STR;
		case Key_F2:           return INPUT_DISP_CHAR_F2_STR;
		case Key_F3:           return INPUT_DISP_CHAR_F3_STR;
		case Key_F4:           return INPUT_DISP_CHAR_F4_STR;
		case Key_F5:           return INPUT_DISP_CHAR_F5_STR;
		case Key_F6:           return INPUT_DISP_CHAR_F6_STR;
		case Key_F7:           return INPUT_DISP_CHAR_F7_STR;
		case Key_F8:           return INPUT_DISP_CHAR_F8_STR;
		case Key_F9:           return INPUT_DISP_CHAR_F9_STR;
		case Key_F10:          return INPUT_DISP_CHAR_F10_STR;
		case Key_F11:          return INPUT_DISP_CHAR_F11_STR;
		case Key_F12:          return INPUT_DISP_CHAR_F12_STR;
		case Key_Space:        return INPUT_DISP_CHAR_SPACE_STR;
		case Key_Tilde:        return INPUT_DISP_CHAR_TILDE_STR;
		case Key_1:            return INPUT_DISP_CHAR_1_STR;
		case Key_2:            return INPUT_DISP_CHAR_2_STR;
		case Key_3:            return INPUT_DISP_CHAR_3_STR;
		case Key_4:            return INPUT_DISP_CHAR_4_STR;
		case Key_5:            return INPUT_DISP_CHAR_5_STR;
		case Key_6:            return INPUT_DISP_CHAR_6_STR;
		case Key_7:            return INPUT_DISP_CHAR_7_STR;
		case Key_8:            return INPUT_DISP_CHAR_8_STR;
		case Key_9:            return INPUT_DISP_CHAR_9_STR;
		case Key_0:            return INPUT_DISP_CHAR_0_STR;
		case Key_Minus:        return INPUT_DISP_CHAR_MINUS_STR;
		case Key_Plus:         return INPUT_DISP_CHAR_PLUS_STR;
		case Key_Backspace:    return INPUT_DISP_CHAR_BACK_STR;
		case Key_Insert:       return INPUT_DISP_CHAR_INSERT_STR;
		case Key_Delete:       return INPUT_DISP_CHAR_DELETE_STR;
		case Key_Tab:          return INPUT_DISP_CHAR_TAB_STR;
		case Key_Q:            return INPUT_DISP_CHAR_Q_STR;
		case Key_W:            return INPUT_DISP_CHAR_W_STR;
		case Key_E:            return INPUT_DISP_CHAR_E_STR;
		case Key_R:            return INPUT_DISP_CHAR_R_STR;
		case Key_T:            return INPUT_DISP_CHAR_T_STR;
		case Key_Y:            return INPUT_DISP_CHAR_Y_STR;
		case Key_U:            return INPUT_DISP_CHAR_U_STR;
		case Key_I:            return INPUT_DISP_CHAR_I_STR;
		case Key_O:            return INPUT_DISP_CHAR_O_STR;
		case Key_P:            return INPUT_DISP_CHAR_P_STR;
		case Key_OpenBracket:  return INPUT_DISP_CHAR_OPEN_BRACKET_STR;
		case Key_CloseBracket: return INPUT_DISP_CHAR_CLOSE_BRACKET_STR;
		case Key_Pipe:         return INPUT_DISP_CHAR_PIPE_STR;
		case Key_Home:         return INPUT_DISP_CHAR_HOME_STR;
		case Key_End:          return INPUT_DISP_CHAR_END_STR;
		case Key_CapsLock:     return INPUT_DISP_CHAR_CAPS_STR;
		case Key_A:            return INPUT_DISP_CHAR_A_STR;
		case Key_S:            return INPUT_DISP_CHAR_S_STR;
		case Key_D:            return INPUT_DISP_CHAR_D_STR;
		case Key_F:            return INPUT_DISP_CHAR_F_STR;
		case Key_G:            return INPUT_DISP_CHAR_G_STR;
		case Key_H:            return INPUT_DISP_CHAR_H_STR;
		case Key_J:            return INPUT_DISP_CHAR_J_STR;
		case Key_K:            return INPUT_DISP_CHAR_K_STR;
		case Key_L:            return INPUT_DISP_CHAR_L_STR;
		case Key_Colon:        return INPUT_DISP_CHAR_COLON_STR;
		case Key_Quote:        return INPUT_DISP_CHAR_QUOTE_STR;
		case Key_Up:           return INPUT_DISP_CHAR_UP_STR;
		case Key_Enter:        return INPUT_DISP_CHAR_ENTER_STR;
		case Key_PageUp:       return INPUT_DISP_CHAR_PAGE_UP_STR;
		case Key_PageDown:     return INPUT_DISP_CHAR_PAGE_DOWN_STR;
		case Key_Shift:        return INPUT_DISP_CHAR_SHIFT_STR;
		case Key_Z:            return INPUT_DISP_CHAR_Z_STR;
		case Key_X:            return INPUT_DISP_CHAR_X_STR;
		case Key_C:            return INPUT_DISP_CHAR_C_STR;
		case Key_V:            return INPUT_DISP_CHAR_V_STR;
		case Key_B:            return INPUT_DISP_CHAR_B_STR;
		case Key_N:            return INPUT_DISP_CHAR_N_STR;
		case Key_M:            return INPUT_DISP_CHAR_M_STR;
		case Key_Comma:        return INPUT_DISP_CHAR_COMMA_STR;
		case Key_Period:       return INPUT_DISP_CHAR_PERIOD_STR;
		case Key_QuestionMark: return INPUT_DISP_CHAR_QUESTION_STR;
		case Key_Left:         return INPUT_DISP_CHAR_LEFT_STR;
		case Key_Down:         return INPUT_DISP_CHAR_DOWN_STR;
		case Key_Right:        return INPUT_DISP_CHAR_RIGHT_STR;
		case Key_Alt:          return INPUT_DISP_CHAR_ALT_STR;
		case Key_Control:      return INPUT_DISP_CHAR_CONTROL_STR;
		default: return INPUT_DISP_CHAR_BLANK_STR;
	}
}
u32 GetDisplayCharCodepointForKey(Key_t key)
{
	switch (key)
	{
		case Key_Escape:       return INPUT_DISP_CHAR_ESCAPE_CODEPOINT;
		case Key_F1:           return INPUT_DISP_CHAR_F1_CODEPOINT;
		case Key_F2:           return INPUT_DISP_CHAR_F2_CODEPOINT;
		case Key_F3:           return INPUT_DISP_CHAR_F3_CODEPOINT;
		case Key_F4:           return INPUT_DISP_CHAR_F4_CODEPOINT;
		case Key_F5:           return INPUT_DISP_CHAR_F5_CODEPOINT;
		case Key_F6:           return INPUT_DISP_CHAR_F6_CODEPOINT;
		case Key_F7:           return INPUT_DISP_CHAR_F7_CODEPOINT;
		case Key_F8:           return INPUT_DISP_CHAR_F8_CODEPOINT;
		case Key_F9:           return INPUT_DISP_CHAR_F9_CODEPOINT;
		case Key_F10:          return INPUT_DISP_CHAR_F10_CODEPOINT;
		case Key_F11:          return INPUT_DISP_CHAR_F11_CODEPOINT;
		case Key_F12:          return INPUT_DISP_CHAR_F12_CODEPOINT;
		case Key_Space:        return INPUT_DISP_CHAR_SPACE_CODEPOINT;
		case Key_Tilde:        return INPUT_DISP_CHAR_TILDE_CODEPOINT;
		case Key_1:            return INPUT_DISP_CHAR_1_CODEPOINT;
		case Key_2:            return INPUT_DISP_CHAR_2_CODEPOINT;
		case Key_3:            return INPUT_DISP_CHAR_3_CODEPOINT;
		case Key_4:            return INPUT_DISP_CHAR_4_CODEPOINT;
		case Key_5:            return INPUT_DISP_CHAR_5_CODEPOINT;
		case Key_6:            return INPUT_DISP_CHAR_6_CODEPOINT;
		case Key_7:            return INPUT_DISP_CHAR_7_CODEPOINT;
		case Key_8:            return INPUT_DISP_CHAR_8_CODEPOINT;
		case Key_9:            return INPUT_DISP_CHAR_9_CODEPOINT;
		case Key_0:            return INPUT_DISP_CHAR_0_CODEPOINT;
		case Key_Minus:        return INPUT_DISP_CHAR_MINUS_CODEPOINT;
		case Key_Plus:         return INPUT_DISP_CHAR_PLUS_CODEPOINT;
		case Key_Backspace:    return INPUT_DISP_CHAR_BACK_CODEPOINT;
		case Key_Insert:       return INPUT_DISP_CHAR_INSERT_CODEPOINT;
		case Key_Delete:       return INPUT_DISP_CHAR_DELETE_CODEPOINT;
		case Key_Tab:          return INPUT_DISP_CHAR_TAB_CODEPOINT;
		case Key_Q:            return INPUT_DISP_CHAR_Q_CODEPOINT;
		case Key_W:            return INPUT_DISP_CHAR_W_CODEPOINT;
		case Key_E:            return INPUT_DISP_CHAR_E_CODEPOINT;
		case Key_R:            return INPUT_DISP_CHAR_R_CODEPOINT;
		case Key_T:            return INPUT_DISP_CHAR_T_CODEPOINT;
		case Key_Y:            return INPUT_DISP_CHAR_Y_CODEPOINT;
		case Key_U:            return INPUT_DISP_CHAR_U_CODEPOINT;
		case Key_I:            return INPUT_DISP_CHAR_I_CODEPOINT;
		case Key_O:            return INPUT_DISP_CHAR_O_CODEPOINT;
		case Key_P:            return INPUT_DISP_CHAR_P_CODEPOINT;
		case Key_OpenBracket:  return INPUT_DISP_CHAR_OPEN_BRACKET_CODEPOINT;
		case Key_CloseBracket: return INPUT_DISP_CHAR_CLOSE_BRACKET_CODEPOINT;
		case Key_Pipe:         return INPUT_DISP_CHAR_PIPE_CODEPOINT;
		case Key_Home:         return INPUT_DISP_CHAR_HOME_CODEPOINT;
		case Key_End:          return INPUT_DISP_CHAR_END_CODEPOINT;
		case Key_CapsLock:     return INPUT_DISP_CHAR_CAPS_CODEPOINT;
		case Key_A:            return INPUT_DISP_CHAR_A_CODEPOINT;
		case Key_S:            return INPUT_DISP_CHAR_S_CODEPOINT;
		case Key_D:            return INPUT_DISP_CHAR_D_CODEPOINT;
		case Key_F:            return INPUT_DISP_CHAR_F_CODEPOINT;
		case Key_G:            return INPUT_DISP_CHAR_G_CODEPOINT;
		case Key_H:            return INPUT_DISP_CHAR_H_CODEPOINT;
		case Key_J:            return INPUT_DISP_CHAR_J_CODEPOINT;
		case Key_K:            return INPUT_DISP_CHAR_K_CODEPOINT;
		case Key_L:            return INPUT_DISP_CHAR_L_CODEPOINT;
		case Key_Colon:        return INPUT_DISP_CHAR_COLON_CODEPOINT;
		case Key_Quote:        return INPUT_DISP_CHAR_QUOTE_CODEPOINT;
		case Key_Up:           return INPUT_DISP_CHAR_UP_CODEPOINT;
		case Key_Enter:        return INPUT_DISP_CHAR_ENTER_CODEPOINT;
		case Key_PageUp:       return INPUT_DISP_CHAR_PAGE_UP_CODEPOINT;
		case Key_PageDown:     return INPUT_DISP_CHAR_PAGE_DOWN_CODEPOINT;
		case Key_Shift:        return INPUT_DISP_CHAR_SHIFT_CODEPOINT;
		case Key_Z:            return INPUT_DISP_CHAR_Z_CODEPOINT;
		case Key_X:            return INPUT_DISP_CHAR_X_CODEPOINT;
		case Key_C:            return INPUT_DISP_CHAR_C_CODEPOINT;
		case Key_V:            return INPUT_DISP_CHAR_V_CODEPOINT;
		case Key_B:            return INPUT_DISP_CHAR_B_CODEPOINT;
		case Key_N:            return INPUT_DISP_CHAR_N_CODEPOINT;
		case Key_M:            return INPUT_DISP_CHAR_M_CODEPOINT;
		case Key_Comma:        return INPUT_DISP_CHAR_COMMA_CODEPOINT;
		case Key_Period:       return INPUT_DISP_CHAR_PERIOD_CODEPOINT;
		case Key_QuestionMark: return INPUT_DISP_CHAR_QUESTION_CODEPOINT;
		case Key_Left:         return INPUT_DISP_CHAR_LEFT_CODEPOINT;
		case Key_Down:         return INPUT_DISP_CHAR_DOWN_CODEPOINT;
		case Key_Right:        return INPUT_DISP_CHAR_RIGHT_CODEPOINT;
		case Key_Alt:          return INPUT_DISP_CHAR_ALT_CODEPOINT;
		case Key_Control:      return INPUT_DISP_CHAR_CONTROL_CODEPOINT;
		default: return INPUT_DISP_CHAR_BLANK_CODEPOINT;
	}
}

const char* GetDisplayCharStrForMouseBtn(MouseBtn_t mouseBtn)
{
	switch (mouseBtn)
	{
		case MouseBtn_Left:   return INPUT_DISP_CHAR_MOUSE_LEFT_STR;
		case MouseBtn_Right:  return INPUT_DISP_CHAR_MOUSE_RIGHT_STR;
		case MouseBtn_Middle: return INPUT_DISP_CHAR_MOUSE_MIDDLE_STR;
		default: return INPUT_DISP_CHAR_MOUSE_BLANK_STR;
	}
}
u32 GetDisplayCharCodepointForMouseBtn(MouseBtn_t mouseBtn)
{
	switch (mouseBtn)
	{
		case MouseBtn_Left:   return INPUT_DISP_CHAR_MOUSE_LEFT_CODEPOINT;
		case MouseBtn_Right:  return INPUT_DISP_CHAR_MOUSE_RIGHT_CODEPOINT;
		case MouseBtn_Middle: return INPUT_DISP_CHAR_MOUSE_MIDDLE_CODEPOINT;
		default: return INPUT_DISP_CHAR_MOUSE_BLANK_CODEPOINT;
	}
}

const char* GetDisplayCharStrForControllerBtn(ControllerType_t type, ControllerBtn_t controllerBtn)
{
	if (type == ControllerType_Playstation)
	{
		switch (controllerBtn)
		{
			case ControllerBtn_A:            return INPUT_DISP_CHAR_PS_CROSS_STR;
			case ControllerBtn_B:            return INPUT_DISP_CHAR_PS_CIRCLE_STR;
			case ControllerBtn_X:            return INPUT_DISP_CHAR_PS_SQUARE_STR;
			case ControllerBtn_Y:            return INPUT_DISP_CHAR_PS_TRIANGLE_STR;
			case ControllerBtn_Right:        return INPUT_DISP_CHAR_PS_RIGHT_STR;
			case ControllerBtn_Left:         return INPUT_DISP_CHAR_PS_LEFT_STR;
			case ControllerBtn_Up:           return INPUT_DISP_CHAR_PS_UP_STR;
			case ControllerBtn_Down:         return INPUT_DISP_CHAR_PS_DOWN_STR;
			case ControllerBtn_LeftBumper:   return INPUT_DISP_CHAR_PS_L1_STR;
			case ControllerBtn_RightBumper:  return INPUT_DISP_CHAR_PS_R1_STR;
			case ControllerBtn_LeftTrigger:  return INPUT_DISP_CHAR_PS_L2_STR;
			case ControllerBtn_RightTrigger: return INPUT_DISP_CHAR_PS_R2_STR;
			case ControllerBtn_LeftStick:    return INPUT_DISP_CHAR_PS_L3_STR;
			case ControllerBtn_RightStick:   return INPUT_DISP_CHAR_PS_R3_STR;
			case ControllerBtn_Start:        return INPUT_DISP_CHAR_PS_OPTIONS_STR;
			case ControllerBtn_Back:         return INPUT_DISP_CHAR_PS_SHARE_STR;
			// case ControllerBtn_lsRight:      //TODO: Implement me!
			// case ControllerBtn_lsLeft:       //TODO: Implement me!
			// case ControllerBtn_lsUp:         //TODO: Implement me!
			// case ControllerBtn_lsDown:       //TODO: Implement me!
			// case ControllerBtn_rsRight:      //TODO: Implement me!
			// case ControllerBtn_rsLeft:       //TODO: Implement me!
			// case ControllerBtn_rsUp:         //TODO: Implement me!
			// case ControllerBtn_rsDown:       //TODO: Implement me!
			default: return INPUT_DISP_CHAR_PS_UNKNOWN_STR;
		}
	}
	else if (type == ControllerType_Nintendo)
	{
		switch (controllerBtn)
		{
			case ControllerBtn_A:            return INPUT_DISP_CHAR_NT_B_STR;
			case ControllerBtn_B:            return INPUT_DISP_CHAR_NT_A_STR;
			case ControllerBtn_X:            return INPUT_DISP_CHAR_NT_Y_STR;
			case ControllerBtn_Y:            return INPUT_DISP_CHAR_NT_X_STR;
			case ControllerBtn_Right:        return INPUT_DISP_CHAR_NT_RIGHT_STR;
			case ControllerBtn_Left:         return INPUT_DISP_CHAR_NT_LEFT_STR;
			case ControllerBtn_Up:           return INPUT_DISP_CHAR_NT_UP_STR;
			case ControllerBtn_Down:         return INPUT_DISP_CHAR_NT_DOWN_STR;
			case ControllerBtn_LeftBumper:   return INPUT_DISP_CHAR_NT_L_STR;
			case ControllerBtn_RightBumper:  return INPUT_DISP_CHAR_NT_R_STR;
			case ControllerBtn_LeftTrigger:  return INPUT_DISP_CHAR_NT_ZL_STR;
			case ControllerBtn_RightTrigger: return INPUT_DISP_CHAR_NT_ZR_STR;
			// case ControllerBtn_LeftStick:    return INPUT_DISP_CHAR_NT_L3_STR;
			// case ControllerBtn_RightStick:   return INPUT_DISP_CHAR_NT_R3_STR;
			case ControllerBtn_Start:        return INPUT_DISP_CHAR_NT_PLUS_STR;
			case ControllerBtn_Back:         return INPUT_DISP_CHAR_NT_MINUS_STR;
			// case ControllerBtn_lsRight:      //TODO: Implement me!
			// case ControllerBtn_lsLeft:       //TODO: Implement me!
			// case ControllerBtn_lsUp:         //TODO: Implement me!
			// case ControllerBtn_lsDown:       //TODO: Implement me!
			// case ControllerBtn_rsRight:      //TODO: Implement me!
			// case ControllerBtn_rsLeft:       //TODO: Implement me!
			// case ControllerBtn_rsUp:         //TODO: Implement me!
			// case ControllerBtn_rsDown:       //TODO: Implement me!
			default: return INPUT_DISP_CHAR_NT_UNKNOWN_STR;
		}
	}
	else //default to Xbox
	{
		switch (controllerBtn)
		{
			case ControllerBtn_A:            return INPUT_DISP_CHAR_XB_A_STR;
			case ControllerBtn_B:            return INPUT_DISP_CHAR_XB_B_STR;
			case ControllerBtn_X:            return INPUT_DISP_CHAR_XB_X_STR;
			case ControllerBtn_Y:            return INPUT_DISP_CHAR_XB_Y_STR;
			case ControllerBtn_Right:        return INPUT_DISP_CHAR_XB_RIGHT_STR;
			case ControllerBtn_Left:         return INPUT_DISP_CHAR_XB_LEFT_STR;
			case ControllerBtn_Up:           return INPUT_DISP_CHAR_XB_UP_STR;
			case ControllerBtn_Down:         return INPUT_DISP_CHAR_XB_DOWN_STR;
			case ControllerBtn_LeftBumper:   return INPUT_DISP_CHAR_XB_LB_STR;
			case ControllerBtn_RightBumper:  return INPUT_DISP_CHAR_XB_RB_STR;
			case ControllerBtn_LeftTrigger:  return INPUT_DISP_CHAR_XB_LT_STR;
			case ControllerBtn_RightTrigger: return INPUT_DISP_CHAR_XB_RT_STR;
			case ControllerBtn_LeftStick:    return INPUT_DISP_CHAR_XB_L3_STR;
			case ControllerBtn_RightStick:   return INPUT_DISP_CHAR_XB_R3_STR;
			case ControllerBtn_Start:        return INPUT_DISP_CHAR_XB_START_STR;
			case ControllerBtn_Back:         return INPUT_DISP_CHAR_XB_BACK_STR;
			// case ControllerBtn_lsRight:      //TODO: Implement me!
			// case ControllerBtn_lsLeft:       //TODO: Implement me!
			// case ControllerBtn_lsUp:         //TODO: Implement me!
			// case ControllerBtn_lsDown:       //TODO: Implement me!
			// case ControllerBtn_rsRight:      //TODO: Implement me!
			// case ControllerBtn_rsLeft:       //TODO: Implement me!
			// case ControllerBtn_rsUp:         //TODO: Implement me!
			// case ControllerBtn_rsDown:       //TODO: Implement me!
			default: return INPUT_DISP_CHAR_XB_UNKNOWN_STR;
		}
	}
}
u32 GetDisplayCharCodepointForControllerBtn(ControllerType_t type, ControllerBtn_t controllerBtn)
{
	if (type == ControllerType_Playstation)
	{
		switch (controllerBtn)
		{
			case ControllerBtn_A:            return INPUT_DISP_CHAR_PS_CROSS_CODEPOINT;
			case ControllerBtn_B:            return INPUT_DISP_CHAR_PS_CIRCLE_CODEPOINT;
			case ControllerBtn_X:            return INPUT_DISP_CHAR_PS_SQUARE_CODEPOINT;
			case ControllerBtn_Y:            return INPUT_DISP_CHAR_PS_TRIANGLE_CODEPOINT;
			case ControllerBtn_Right:        return INPUT_DISP_CHAR_PS_RIGHT_CODEPOINT;
			case ControllerBtn_Left:         return INPUT_DISP_CHAR_PS_LEFT_CODEPOINT;
			case ControllerBtn_Up:           return INPUT_DISP_CHAR_PS_UP_CODEPOINT;
			case ControllerBtn_Down:         return INPUT_DISP_CHAR_PS_DOWN_CODEPOINT;
			case ControllerBtn_LeftBumper:   return INPUT_DISP_CHAR_PS_L1_CODEPOINT;
			case ControllerBtn_RightBumper:  return INPUT_DISP_CHAR_PS_R1_CODEPOINT;
			case ControllerBtn_LeftTrigger:  return INPUT_DISP_CHAR_PS_L2_CODEPOINT;
			case ControllerBtn_RightTrigger: return INPUT_DISP_CHAR_PS_R2_CODEPOINT;
			case ControllerBtn_LeftStick:    return INPUT_DISP_CHAR_PS_L3_CODEPOINT;
			case ControllerBtn_RightStick:   return INPUT_DISP_CHAR_PS_R3_CODEPOINT;
			case ControllerBtn_Start:        return INPUT_DISP_CHAR_PS_OPTIONS_CODEPOINT;
			case ControllerBtn_Back:         return INPUT_DISP_CHAR_PS_SHARE_CODEPOINT;
			// case ControllerBtn_lsRight:      //TODO: Implement me!
			// case ControllerBtn_lsLeft:       //TODO: Implement me!
			// case ControllerBtn_lsUp:         //TODO: Implement me!
			// case ControllerBtn_lsDown:       //TODO: Implement me!
			// case ControllerBtn_rsRight:      //TODO: Implement me!
			// case ControllerBtn_rsLeft:       //TODO: Implement me!
			// case ControllerBtn_rsUp:         //TODO: Implement me!
			// case ControllerBtn_rsDown:       //TODO: Implement me!
			default: return INPUT_DISP_CHAR_PS_UNKNOWN_CODEPOINT;
		}
	}
	else if (type == ControllerType_Nintendo)
	{
		switch (controllerBtn)
		{
			case ControllerBtn_A:            return INPUT_DISP_CHAR_NT_B_CODEPOINT;
			case ControllerBtn_B:            return INPUT_DISP_CHAR_NT_A_CODEPOINT;
			case ControllerBtn_X:            return INPUT_DISP_CHAR_NT_Y_CODEPOINT;
			case ControllerBtn_Y:            return INPUT_DISP_CHAR_NT_X_CODEPOINT;
			case ControllerBtn_Right:        return INPUT_DISP_CHAR_NT_RIGHT_CODEPOINT;
			case ControllerBtn_Left:         return INPUT_DISP_CHAR_NT_LEFT_CODEPOINT;
			case ControllerBtn_Up:           return INPUT_DISP_CHAR_NT_UP_CODEPOINT;
			case ControllerBtn_Down:         return INPUT_DISP_CHAR_NT_DOWN_CODEPOINT;
			case ControllerBtn_LeftBumper:   return INPUT_DISP_CHAR_NT_L_CODEPOINT;
			case ControllerBtn_RightBumper:  return INPUT_DISP_CHAR_NT_R_CODEPOINT;
			case ControllerBtn_LeftTrigger:  return INPUT_DISP_CHAR_NT_ZL_CODEPOINT;
			case ControllerBtn_RightTrigger: return INPUT_DISP_CHAR_NT_ZR_CODEPOINT;
			// case ControllerBtn_LeftStick:    return INPUT_DISP_CHAR_NT_L3_CODEPOINT;
			// case ControllerBtn_RightStick:   return INPUT_DISP_CHAR_NT_R3_CODEPOINT;
			case ControllerBtn_Start:        return INPUT_DISP_CHAR_NT_PLUS_CODEPOINT;
			case ControllerBtn_Back:         return INPUT_DISP_CHAR_NT_MINUS_CODEPOINT;
			// case ControllerBtn_lsRight:      //TODO: Implement me!
			// case ControllerBtn_lsLeft:       //TODO: Implement me!
			// case ControllerBtn_lsUp:         //TODO: Implement me!
			// case ControllerBtn_lsDown:       //TODO: Implement me!
			// case ControllerBtn_rsRight:      //TODO: Implement me!
			// case ControllerBtn_rsLeft:       //TODO: Implement me!
			// case ControllerBtn_rsUp:         //TODO: Implement me!
			// case ControllerBtn_rsDown:       //TODO: Implement me!
			default: return INPUT_DISP_CHAR_NT_UNKNOWN_CODEPOINT;
		}
	}
	else //default to Xbox
	{
		switch (controllerBtn)
		{
			case ControllerBtn_A:            return INPUT_DISP_CHAR_XB_A_CODEPOINT;
			case ControllerBtn_B:            return INPUT_DISP_CHAR_XB_B_CODEPOINT;
			case ControllerBtn_X:            return INPUT_DISP_CHAR_XB_X_CODEPOINT;
			case ControllerBtn_Y:            return INPUT_DISP_CHAR_XB_Y_CODEPOINT;
			case ControllerBtn_Right:        return INPUT_DISP_CHAR_XB_RIGHT_CODEPOINT;
			case ControllerBtn_Left:         return INPUT_DISP_CHAR_XB_LEFT_CODEPOINT;
			case ControllerBtn_Up:           return INPUT_DISP_CHAR_XB_UP_CODEPOINT;
			case ControllerBtn_Down:         return INPUT_DISP_CHAR_XB_DOWN_CODEPOINT;
			case ControllerBtn_LeftBumper:   return INPUT_DISP_CHAR_XB_LB_CODEPOINT;
			case ControllerBtn_RightBumper:  return INPUT_DISP_CHAR_XB_RB_CODEPOINT;
			case ControllerBtn_LeftTrigger:  return INPUT_DISP_CHAR_XB_LT_CODEPOINT;
			case ControllerBtn_RightTrigger: return INPUT_DISP_CHAR_XB_RT_CODEPOINT;
			case ControllerBtn_LeftStick:    return INPUT_DISP_CHAR_XB_L3_CODEPOINT;
			case ControllerBtn_RightStick:   return INPUT_DISP_CHAR_XB_R3_CODEPOINT;
			case ControllerBtn_Start:        return INPUT_DISP_CHAR_XB_START_CODEPOINT;
			case ControllerBtn_Back:         return INPUT_DISP_CHAR_XB_BACK_CODEPOINT;
			// case ControllerBtn_lsRight:      //TODO: Implement me!
			// case ControllerBtn_lsLeft:       //TODO: Implement me!
			// case ControllerBtn_lsUp:         //TODO: Implement me!
			// case ControllerBtn_lsDown:       //TODO: Implement me!
			// case ControllerBtn_rsRight:      //TODO: Implement me!
			// case ControllerBtn_rsLeft:       //TODO: Implement me!
			// case ControllerBtn_rsUp:         //TODO: Implement me!
			// case ControllerBtn_rsDown:       //TODO: Implement me!
			default: return INPUT_DISP_CHAR_XB_UNKNOWN_CODEPOINT;
		}
	}
}

#endif //  _PIG_INPUT_CHARS_H
