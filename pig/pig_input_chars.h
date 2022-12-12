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

#define NUM_INPUT_DISP_CHAR_CODEPOINTS  147

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

u32 GetInputCharCodepointByIndex(u64 index)
{
	switch (index)
	{
		case 0:   return INPUT_DISP_CHAR_ESCAPE_CODEPOINT;
		case 1:   return INPUT_DISP_CHAR_F1_CODEPOINT;
		case 2:   return INPUT_DISP_CHAR_F2_CODEPOINT;
		case 3:   return INPUT_DISP_CHAR_F3_CODEPOINT;
		case 4:   return INPUT_DISP_CHAR_F4_CODEPOINT;
		case 5:   return INPUT_DISP_CHAR_F5_CODEPOINT;
		case 6:   return INPUT_DISP_CHAR_F6_CODEPOINT;
		case 7:   return INPUT_DISP_CHAR_F7_CODEPOINT;
		case 8:   return INPUT_DISP_CHAR_F8_CODEPOINT;
		case 9:   return INPUT_DISP_CHAR_F9_CODEPOINT;
		case 10:  return INPUT_DISP_CHAR_F10_CODEPOINT;
		case 11:  return INPUT_DISP_CHAR_F11_CODEPOINT;
		case 12:  return INPUT_DISP_CHAR_F12_CODEPOINT;
		case 13:  return INPUT_DISP_CHAR_SPACE_CODEPOINT;
		case 14:  return INPUT_DISP_CHAR_BLANK_CODEPOINT;
		case 15:  return INPUT_DISP_CHAR_BLANK_WIDE_CODEPOINT;
		case 16:  return INPUT_DISP_CHAR_TILDE_CODEPOINT;
		case 17:  return INPUT_DISP_CHAR_1_CODEPOINT;
		case 18:  return INPUT_DISP_CHAR_2_CODEPOINT;
		case 19:  return INPUT_DISP_CHAR_3_CODEPOINT;
		case 20:  return INPUT_DISP_CHAR_4_CODEPOINT;
		case 21:  return INPUT_DISP_CHAR_5_CODEPOINT;
		case 22:  return INPUT_DISP_CHAR_6_CODEPOINT;
		case 23:  return INPUT_DISP_CHAR_7_CODEPOINT;
		case 24:  return INPUT_DISP_CHAR_8_CODEPOINT;
		case 25:  return INPUT_DISP_CHAR_9_CODEPOINT;
		case 26:  return INPUT_DISP_CHAR_0_CODEPOINT;
		case 27:  return INPUT_DISP_CHAR_MINUS_CODEPOINT;
		case 28:  return INPUT_DISP_CHAR_PLUS_CODEPOINT;
		case 29:  return INPUT_DISP_CHAR_BACK_CODEPOINT;
		case 30:  return INPUT_DISP_CHAR_INSERT_CODEPOINT;
		case 31:  return INPUT_DISP_CHAR_DELETE_CODEPOINT;
		case 32:  return INPUT_DISP_CHAR_TAB_CODEPOINT;
		case 33:  return INPUT_DISP_CHAR_Q_CODEPOINT;
		case 34:  return INPUT_DISP_CHAR_W_CODEPOINT;
		case 35:  return INPUT_DISP_CHAR_E_CODEPOINT;
		case 36:  return INPUT_DISP_CHAR_R_CODEPOINT;
		case 37:  return INPUT_DISP_CHAR_T_CODEPOINT;
		case 38:  return INPUT_DISP_CHAR_Y_CODEPOINT;
		case 39:  return INPUT_DISP_CHAR_U_CODEPOINT;
		case 40:  return INPUT_DISP_CHAR_I_CODEPOINT;
		case 41:  return INPUT_DISP_CHAR_O_CODEPOINT;
		case 42:  return INPUT_DISP_CHAR_P_CODEPOINT;
		case 43:  return INPUT_DISP_CHAR_OPEN_BRACKET_CODEPOINT;
		case 44:  return INPUT_DISP_CHAR_CLOSE_BRACKET_CODEPOINT;
		case 45:  return INPUT_DISP_CHAR_PIPE_CODEPOINT;
		case 46:  return INPUT_DISP_CHAR_HOME_CODEPOINT;
		case 47:  return INPUT_DISP_CHAR_END_CODEPOINT;
		case 48:  return INPUT_DISP_CHAR_CAPS_CODEPOINT;
		case 49:  return INPUT_DISP_CHAR_A_CODEPOINT;
		case 50:  return INPUT_DISP_CHAR_S_CODEPOINT;
		case 51:  return INPUT_DISP_CHAR_D_CODEPOINT;
		case 52:  return INPUT_DISP_CHAR_F_CODEPOINT;
		case 53:  return INPUT_DISP_CHAR_G_CODEPOINT;
		case 54:  return INPUT_DISP_CHAR_H_CODEPOINT;
		case 55:  return INPUT_DISP_CHAR_J_CODEPOINT;
		case 56:  return INPUT_DISP_CHAR_K_CODEPOINT;
		case 57:  return INPUT_DISP_CHAR_L_CODEPOINT;
		case 58:  return INPUT_DISP_CHAR_COLON_CODEPOINT;
		case 59:  return INPUT_DISP_CHAR_QUOTE_CODEPOINT;
		case 60:  return INPUT_DISP_CHAR_UP_CODEPOINT;
		case 61:  return INPUT_DISP_CHAR_ENTER_CODEPOINT;
		case 62:  return INPUT_DISP_CHAR_PAGE_UP_CODEPOINT;
		case 63:  return INPUT_DISP_CHAR_PAGE_DOWN_CODEPOINT;
		case 64:  return INPUT_DISP_CHAR_SHIFT_CODEPOINT;
		case 65:  return INPUT_DISP_CHAR_Z_CODEPOINT;
		case 66:  return INPUT_DISP_CHAR_X_CODEPOINT;
		case 67:  return INPUT_DISP_CHAR_C_CODEPOINT;
		case 68:  return INPUT_DISP_CHAR_V_CODEPOINT;
		case 69:  return INPUT_DISP_CHAR_B_CODEPOINT;
		case 70:  return INPUT_DISP_CHAR_N_CODEPOINT;
		case 71:  return INPUT_DISP_CHAR_M_CODEPOINT;
		case 72:  return INPUT_DISP_CHAR_COMMA_CODEPOINT;
		case 73:  return INPUT_DISP_CHAR_PERIOD_CODEPOINT;
		case 74:  return INPUT_DISP_CHAR_QUESTION_CODEPOINT;
		case 75:  return INPUT_DISP_CHAR_LEFT_CODEPOINT;
		case 76:  return INPUT_DISP_CHAR_DOWN_CODEPOINT;
		case 77:  return INPUT_DISP_CHAR_RIGHT_CODEPOINT;
		case 78:  return INPUT_DISP_CHAR_ALT_CODEPOINT;
		case 79:  return INPUT_DISP_CHAR_CONTROL_CODEPOINT;
		case 80:  return INPUT_DISP_CHAR_BLANK_SQUARE_CODEPOINT;
		case 81:  return INPUT_DISP_CHAR_BLANK_SQUARE_LARGE_CODEPOINT;
		case 82:  return INPUT_DISP_CHAR_BLANK_SEMI_WIDE_CODEPOINT;
		case 83:  return INPUT_DISP_CHAR_MOUSE_BLANK_CODEPOINT;
		case 84:  return INPUT_DISP_CHAR_MOUSE_LEFT_CODEPOINT;
		case 85:  return INPUT_DISP_CHAR_MOUSE_RIGHT_CODEPOINT;
		case 86:  return INPUT_DISP_CHAR_MOUSE_MIDDLE_CODEPOINT;
		case 87:  return INPUT_DISP_CHAR_XB_A_CODEPOINT;
		case 88:  return INPUT_DISP_CHAR_XB_B_CODEPOINT;
		case 89:  return INPUT_DISP_CHAR_XB_X_CODEPOINT;
		case 90:  return INPUT_DISP_CHAR_XB_Y_CODEPOINT;
		case 91:  return INPUT_DISP_CHAR_XB_LB_CODEPOINT;
		case 92:  return INPUT_DISP_CHAR_XB_RB_CODEPOINT;
		case 93:  return INPUT_DISP_CHAR_XB_LT_CODEPOINT;
		case 94:  return INPUT_DISP_CHAR_XB_RT_CODEPOINT;
		case 95:  return INPUT_DISP_CHAR_XB_L3_CODEPOINT;
		case 96:  return INPUT_DISP_CHAR_XB_R3_CODEPOINT;
		case 97:  return INPUT_DISP_CHAR_XB_BACK_CODEPOINT;
		case 98:  return INPUT_DISP_CHAR_XB_START_CODEPOINT;
		case 99:  return INPUT_DISP_CHAR_XB_UP_CODEPOINT;
		case 100: return INPUT_DISP_CHAR_XB_LEFT_CODEPOINT;
		case 101: return INPUT_DISP_CHAR_XB_DOWN_CODEPOINT;
		case 102: return INPUT_DISP_CHAR_XB_RIGHT_CODEPOINT;
		case 103: return INPUT_DISP_CHAR_XB_LEFT_STICK_CODEPOINT;
		case 104: return INPUT_DISP_CHAR_XB_RIGHT_STICK_CODEPOINT;
		case 105: return INPUT_DISP_CHAR_XB_DPAD_CODEPOINT;
		case 106: return INPUT_DISP_CHAR_XB_UNKNOWN_CODEPOINT;
		case 107: return INPUT_DISP_CHAR_PS_CROSS_CODEPOINT;
		case 108: return INPUT_DISP_CHAR_PS_CIRCLE_CODEPOINT;
		case 109: return INPUT_DISP_CHAR_PS_SQUARE_CODEPOINT;
		case 110: return INPUT_DISP_CHAR_PS_TRIANGLE_CODEPOINT;
		case 111: return INPUT_DISP_CHAR_PS_L1_CODEPOINT;
		case 112: return INPUT_DISP_CHAR_PS_R1_CODEPOINT;
		case 113: return INPUT_DISP_CHAR_PS_L2_CODEPOINT;
		case 114: return INPUT_DISP_CHAR_PS_R2_CODEPOINT;
		case 115: return INPUT_DISP_CHAR_PS_L3_CODEPOINT;
		case 116: return INPUT_DISP_CHAR_PS_R3_CODEPOINT;
		case 117: return INPUT_DISP_CHAR_PS_SHARE_CODEPOINT;
		case 118: return INPUT_DISP_CHAR_PS_OPTIONS_CODEPOINT;
		case 119: return INPUT_DISP_CHAR_PS_UP_CODEPOINT;
		case 120: return INPUT_DISP_CHAR_PS_LEFT_CODEPOINT;
		case 121: return INPUT_DISP_CHAR_PS_DOWN_CODEPOINT;
		case 122: return INPUT_DISP_CHAR_PS_RIGHT_CODEPOINT;
		case 123: return INPUT_DISP_CHAR_PS_LEFT_STICK_CODEPOINT;
		case 124: return INPUT_DISP_CHAR_PS_RIGHT_STICK_CODEPOINT;
		case 125: return INPUT_DISP_CHAR_PS_DPAD_CODEPOINT;
		case 126: return INPUT_DISP_CHAR_PS_UNKNOWN_CODEPOINT;
		case 127: return INPUT_DISP_CHAR_NT_B_CODEPOINT;
		case 128: return INPUT_DISP_CHAR_NT_A_CODEPOINT;
		case 129: return INPUT_DISP_CHAR_NT_Y_CODEPOINT;
		case 130: return INPUT_DISP_CHAR_NT_X_CODEPOINT;
		case 131: return INPUT_DISP_CHAR_NT_L_CODEPOINT;
		case 132: return INPUT_DISP_CHAR_NT_R_CODEPOINT;
		case 133: return INPUT_DISP_CHAR_NT_ZL_CODEPOINT;
		case 134: return INPUT_DISP_CHAR_NT_ZR_CODEPOINT;
		case 135: return 0; //INPUT_DISP_CHAR_NT_L3_CODEPOINT;
		case 136: return 0; //INPUT_DISP_CHAR_NT_R3_CODEPOINT;
		case 137: return INPUT_DISP_CHAR_NT_MINUS_CODEPOINT;
		case 138: return INPUT_DISP_CHAR_NT_PLUS_CODEPOINT;
		case 139: return INPUT_DISP_CHAR_NT_UP_CODEPOINT;
		case 140: return INPUT_DISP_CHAR_NT_LEFT_CODEPOINT;
		case 141: return INPUT_DISP_CHAR_NT_DOWN_CODEPOINT;
		case 142: return INPUT_DISP_CHAR_NT_RIGHT_CODEPOINT;
		case 143: return INPUT_DISP_CHAR_NT_LEFT_STICK_CODEPOINT;
		case 144: return INPUT_DISP_CHAR_NT_RIGHT_STICK_CODEPOINT;
		case 145: return INPUT_DISP_CHAR_NT_DPAD_CODEPOINT;
		case 146: return INPUT_DISP_CHAR_NT_UNKNOWN_CODEPOINT;
		default: return 0;
	}
}

const char* GetInputCharNameByCodepoint(u32 codepoint)
{
	switch (codepoint)
	{
		case INPUT_DISP_CHAR_ESCAPE_CODEPOINT:             return "ESCAPE";
		case INPUT_DISP_CHAR_F1_CODEPOINT:                 return "F1";
		case INPUT_DISP_CHAR_F2_CODEPOINT:                 return "F2";
		case INPUT_DISP_CHAR_F3_CODEPOINT:                 return "F3";
		case INPUT_DISP_CHAR_F4_CODEPOINT:                 return "F4";
		case INPUT_DISP_CHAR_F5_CODEPOINT:                 return "F5";
		case INPUT_DISP_CHAR_F6_CODEPOINT:                 return "F6";
		case INPUT_DISP_CHAR_F7_CODEPOINT:                 return "F7";
		case INPUT_DISP_CHAR_F8_CODEPOINT:                 return "F8";
		case INPUT_DISP_CHAR_F9_CODEPOINT:                 return "F9";
		case INPUT_DISP_CHAR_F10_CODEPOINT:                return "F10";
		case INPUT_DISP_CHAR_F11_CODEPOINT:                return "F11";
		case INPUT_DISP_CHAR_F12_CODEPOINT:                return "F12";
		case INPUT_DISP_CHAR_SPACE_CODEPOINT:              return "SPACE";
		case INPUT_DISP_CHAR_BLANK_CODEPOINT:              return "BLANK";
		case INPUT_DISP_CHAR_BLANK_WIDE_CODEPOINT:         return "BLANK_WIDE";
		case INPUT_DISP_CHAR_TILDE_CODEPOINT:              return "TILDE";
		case INPUT_DISP_CHAR_1_CODEPOINT:                  return "1";
		case INPUT_DISP_CHAR_2_CODEPOINT:                  return "2";
		case INPUT_DISP_CHAR_3_CODEPOINT:                  return "3";
		case INPUT_DISP_CHAR_4_CODEPOINT:                  return "4";
		case INPUT_DISP_CHAR_5_CODEPOINT:                  return "5";
		case INPUT_DISP_CHAR_6_CODEPOINT:                  return "6";
		case INPUT_DISP_CHAR_7_CODEPOINT:                  return "7";
		case INPUT_DISP_CHAR_8_CODEPOINT:                  return "8";
		case INPUT_DISP_CHAR_9_CODEPOINT:                  return "9";
		case INPUT_DISP_CHAR_0_CODEPOINT:                  return "0";
		case INPUT_DISP_CHAR_MINUS_CODEPOINT:              return "MINUS";
		case INPUT_DISP_CHAR_PLUS_CODEPOINT:               return "PLUS";
		case INPUT_DISP_CHAR_BACK_CODEPOINT:               return "BACK";
		case INPUT_DISP_CHAR_INSERT_CODEPOINT:             return "INSERT";
		case INPUT_DISP_CHAR_DELETE_CODEPOINT:             return "DELETE";
		case INPUT_DISP_CHAR_TAB_CODEPOINT:                return "TAB";
		case INPUT_DISP_CHAR_Q_CODEPOINT:                  return "Q";
		case INPUT_DISP_CHAR_W_CODEPOINT:                  return "W";
		case INPUT_DISP_CHAR_E_CODEPOINT:                  return "E";
		case INPUT_DISP_CHAR_R_CODEPOINT:                  return "R";
		case INPUT_DISP_CHAR_T_CODEPOINT:                  return "T";
		case INPUT_DISP_CHAR_Y_CODEPOINT:                  return "Y";
		case INPUT_DISP_CHAR_U_CODEPOINT:                  return "U";
		case INPUT_DISP_CHAR_I_CODEPOINT:                  return "I";
		case INPUT_DISP_CHAR_O_CODEPOINT:                  return "O";
		case INPUT_DISP_CHAR_P_CODEPOINT:                  return "P";
		case INPUT_DISP_CHAR_OPEN_BRACKET_CODEPOINT:       return "OPEN_BRACKET";
		case INPUT_DISP_CHAR_CLOSE_BRACKET_CODEPOINT:      return "CLOSE_BRACKET";
		case INPUT_DISP_CHAR_PIPE_CODEPOINT:               return "PIPE";
		case INPUT_DISP_CHAR_HOME_CODEPOINT:               return "HOME";
		case INPUT_DISP_CHAR_END_CODEPOINT:                return "END";
		case INPUT_DISP_CHAR_CAPS_CODEPOINT:               return "CAPS";
		case INPUT_DISP_CHAR_A_CODEPOINT:                  return "A";
		case INPUT_DISP_CHAR_S_CODEPOINT:                  return "S";
		case INPUT_DISP_CHAR_D_CODEPOINT:                  return "D";
		case INPUT_DISP_CHAR_F_CODEPOINT:                  return "F";
		case INPUT_DISP_CHAR_G_CODEPOINT:                  return "G";
		case INPUT_DISP_CHAR_H_CODEPOINT:                  return "H";
		case INPUT_DISP_CHAR_J_CODEPOINT:                  return "J";
		case INPUT_DISP_CHAR_K_CODEPOINT:                  return "K";
		case INPUT_DISP_CHAR_L_CODEPOINT:                  return "L";
		case INPUT_DISP_CHAR_COLON_CODEPOINT:              return "COLON";
		case INPUT_DISP_CHAR_QUOTE_CODEPOINT:              return "QUOTE";
		case INPUT_DISP_CHAR_UP_CODEPOINT:                 return "UP";
		case INPUT_DISP_CHAR_ENTER_CODEPOINT:              return "ENTER";
		case INPUT_DISP_CHAR_PAGE_UP_CODEPOINT:            return "PAGE_UP";
		case INPUT_DISP_CHAR_PAGE_DOWN_CODEPOINT:          return "PAGE_DOWN";
		case INPUT_DISP_CHAR_SHIFT_CODEPOINT:              return "SHIFT";
		case INPUT_DISP_CHAR_Z_CODEPOINT:                  return "Z";
		case INPUT_DISP_CHAR_X_CODEPOINT:                  return "X";
		case INPUT_DISP_CHAR_C_CODEPOINT:                  return "C";
		case INPUT_DISP_CHAR_V_CODEPOINT:                  return "V";
		case INPUT_DISP_CHAR_B_CODEPOINT:                  return "B";
		case INPUT_DISP_CHAR_N_CODEPOINT:                  return "N";
		case INPUT_DISP_CHAR_M_CODEPOINT:                  return "M";
		case INPUT_DISP_CHAR_COMMA_CODEPOINT:              return "COMMA";
		case INPUT_DISP_CHAR_PERIOD_CODEPOINT:             return "PERIOD";
		case INPUT_DISP_CHAR_QUESTION_CODEPOINT:           return "QUESTION";
		case INPUT_DISP_CHAR_LEFT_CODEPOINT:               return "LEFT";
		case INPUT_DISP_CHAR_DOWN_CODEPOINT:               return "DOWN";
		case INPUT_DISP_CHAR_RIGHT_CODEPOINT:              return "RIGHT";
		case INPUT_DISP_CHAR_ALT_CODEPOINT:                return "ALT";
		case INPUT_DISP_CHAR_CONTROL_CODEPOINT:            return "CONTROL";
		case INPUT_DISP_CHAR_BLANK_SQUARE_CODEPOINT:       return "BLANK_SQUARE";
		case INPUT_DISP_CHAR_BLANK_SQUARE_LARGE_CODEPOINT: return "BLANK_SQUARE_LARGE";
		case INPUT_DISP_CHAR_BLANK_SEMI_WIDE_CODEPOINT:    return "BLANK_SEMI_WIDE";
		case INPUT_DISP_CHAR_MOUSE_BLANK_CODEPOINT:        return "MOUSE_BLANK";
		case INPUT_DISP_CHAR_MOUSE_LEFT_CODEPOINT:         return "MOUSE_LEFT";
		case INPUT_DISP_CHAR_MOUSE_RIGHT_CODEPOINT:        return "MOUSE_RIGHT";
		case INPUT_DISP_CHAR_MOUSE_MIDDLE_CODEPOINT:       return "MOUSE_MIDDLE";
		case INPUT_DISP_CHAR_XB_A_CODEPOINT:               return "XB_A";
		case INPUT_DISP_CHAR_XB_B_CODEPOINT:               return "XB_B";
		case INPUT_DISP_CHAR_XB_X_CODEPOINT:               return "XB_X";
		case INPUT_DISP_CHAR_XB_Y_CODEPOINT:               return "XB_Y";
		case INPUT_DISP_CHAR_XB_LB_CODEPOINT:              return "XB_LB";
		case INPUT_DISP_CHAR_XB_RB_CODEPOINT:              return "XB_RB";
		case INPUT_DISP_CHAR_XB_LT_CODEPOINT:              return "XB_LT";
		case INPUT_DISP_CHAR_XB_RT_CODEPOINT:              return "XB_RT";
		case INPUT_DISP_CHAR_XB_L3_CODEPOINT:              return "XB_L3";
		case INPUT_DISP_CHAR_XB_R3_CODEPOINT:              return "XB_R3";
		case INPUT_DISP_CHAR_XB_BACK_CODEPOINT:            return "XB_BACK";
		case INPUT_DISP_CHAR_XB_START_CODEPOINT:           return "XB_START";
		case INPUT_DISP_CHAR_XB_UP_CODEPOINT:              return "XB_UP";
		case INPUT_DISP_CHAR_XB_LEFT_CODEPOINT:            return "XB_LEFT";
		case INPUT_DISP_CHAR_XB_DOWN_CODEPOINT:            return "XB_DOWN";
		case INPUT_DISP_CHAR_XB_RIGHT_CODEPOINT:           return "XB_RIGHT";
		case INPUT_DISP_CHAR_XB_LEFT_STICK_CODEPOINT:      return "XB_LEFT_STICK";
		case INPUT_DISP_CHAR_XB_RIGHT_STICK_CODEPOINT:     return "XB_RIGHT_STICK";
		case INPUT_DISP_CHAR_XB_DPAD_CODEPOINT:            return "XB_DPAD";
		case INPUT_DISP_CHAR_XB_UNKNOWN_CODEPOINT:         return "XB_UNKNOWN";
		case INPUT_DISP_CHAR_PS_CROSS_CODEPOINT:           return "PS_CROSS";
		case INPUT_DISP_CHAR_PS_CIRCLE_CODEPOINT:          return "PS_CIRCLE";
		case INPUT_DISP_CHAR_PS_SQUARE_CODEPOINT:          return "PS_SQUARE";
		case INPUT_DISP_CHAR_PS_TRIANGLE_CODEPOINT:        return "PS_TRIANGLE";
		case INPUT_DISP_CHAR_PS_L1_CODEPOINT:              return "PS_L1";
		case INPUT_DISP_CHAR_PS_R1_CODEPOINT:              return "PS_R1";
		case INPUT_DISP_CHAR_PS_L2_CODEPOINT:              return "PS_L2";
		case INPUT_DISP_CHAR_PS_R2_CODEPOINT:              return "PS_R2";
		case INPUT_DISP_CHAR_PS_L3_CODEPOINT:              return "PS_L3";
		case INPUT_DISP_CHAR_PS_R3_CODEPOINT:              return "PS_R3";
		case INPUT_DISP_CHAR_PS_SHARE_CODEPOINT:           return "PS_SHARE";
		case INPUT_DISP_CHAR_PS_OPTIONS_CODEPOINT:         return "PS_OPTIONS";
		case INPUT_DISP_CHAR_PS_UP_CODEPOINT:              return "PS_UP";
		case INPUT_DISP_CHAR_PS_LEFT_CODEPOINT:            return "PS_LEFT";
		case INPUT_DISP_CHAR_PS_DOWN_CODEPOINT:            return "PS_DOWN";
		case INPUT_DISP_CHAR_PS_RIGHT_CODEPOINT:           return "PS_RIGHT";
		case INPUT_DISP_CHAR_PS_LEFT_STICK_CODEPOINT:      return "PS_LEFT_STICK";
		case INPUT_DISP_CHAR_PS_RIGHT_STICK_CODEPOINT:     return "PS_RIGHT_STICK";
		case INPUT_DISP_CHAR_PS_DPAD_CODEPOINT:            return "PS_DPAD";
		case INPUT_DISP_CHAR_PS_UNKNOWN_CODEPOINT:         return "PS_UNKNOWN";
		case INPUT_DISP_CHAR_NT_B_CODEPOINT:               return "NT_B";
		case INPUT_DISP_CHAR_NT_A_CODEPOINT:               return "NT_A";
		case INPUT_DISP_CHAR_NT_Y_CODEPOINT:               return "NT_Y";
		case INPUT_DISP_CHAR_NT_X_CODEPOINT:               return "NT_X";
		case INPUT_DISP_CHAR_NT_L_CODEPOINT:               return "NT_L";
		case INPUT_DISP_CHAR_NT_R_CODEPOINT:               return "NT_R";
		case INPUT_DISP_CHAR_NT_ZL_CODEPOINT:              return "NT_ZL";
		case INPUT_DISP_CHAR_NT_ZR_CODEPOINT:              return "NT_ZR";
		// case INPUT_DISP_CHAR_NT_L3_CODEPOINT:              return "NT_L3";
		// case INPUT_DISP_CHAR_NT_R3_CODEPOINT:              return "NT_R3";
		case INPUT_DISP_CHAR_NT_MINUS_CODEPOINT:           return "NT_MINUS";
		case INPUT_DISP_CHAR_NT_PLUS_CODEPOINT:            return "NT_PLUS";
		case INPUT_DISP_CHAR_NT_UP_CODEPOINT:              return "NT_UP";
		case INPUT_DISP_CHAR_NT_LEFT_CODEPOINT:            return "NT_LEFT";
		case INPUT_DISP_CHAR_NT_DOWN_CODEPOINT:            return "NT_DOWN";
		case INPUT_DISP_CHAR_NT_RIGHT_CODEPOINT:           return "NT_RIGHT";
		case INPUT_DISP_CHAR_NT_LEFT_STICK_CODEPOINT:      return "NT_LEFT_STICK";
		case INPUT_DISP_CHAR_NT_RIGHT_STICK_CODEPOINT:     return "NT_RIGHT_STICK";
		case INPUT_DISP_CHAR_NT_DPAD_CODEPOINT:            return "NT_DPAD";
		case INPUT_DISP_CHAR_NT_UNKNOWN_CODEPOINT:         return "NT_UNKNOWN";
		default: return "Unknown";
	}
}

MyStr_t ParseInputCharSyntax(MyStr_t input, MemArena_t* memArena, u64* numItemsParsedOut = nullptr)
{
	NotNullStr(&input);
	
	MyStr_t result = MyStr_Empty;
	for (u8 pass = 0; pass < 2; pass++)
	{
		u64 numItemsParsed = 0;
		u64 outputByteIndex = 0;
		for (u64 cIndex = 0; cIndex < input.length; )
		{
			u32 codepoint = 0;
			u8 codepointSize = GetCodepointForUtf8Str(input, cIndex, &codepoint);
			if (codepointSize == 0)
			{
				codepointSize = 1;
				codepoint = CharToU32(input.chars[cIndex]);
			}
			
			bool parsedFoundText = false;
			if (codepoint == '{')
			{
				u64 closingCurlyIndex = 0;
				if (FindNextCharInStr(input, cIndex, "}", &closingCurlyIndex))
				{
					Assert(closingCurlyIndex >= cIndex + codepointSize);
					MyStr_t foundText = NewStr(closingCurlyIndex - (cIndex + codepointSize), &input.chars[cIndex + codepointSize]);
					
					for (u64 iIndex = 0; iIndex < NUM_INPUT_DISP_CHAR_CODEPOINTS; iIndex++)
					{
						u32 inputDispCodepoint = GetInputCharCodepointByIndex(iIndex);
						if (inputDispCodepoint != 0)
						{
							const char* inputDispCodepointName = GetInputCharNameByCodepoint(inputDispCodepoint);
							if (StrEqualsIgnoreCase(foundText, inputDispCodepointName))
							{
								parsedFoundText = true;
								numItemsParsed++;
								u8 inputDispCharSize = GetUtf8BytesForCode(inputDispCodepoint, nullptr);
								if (result.chars != nullptr)
								{
									Assert(outputByteIndex + inputDispCharSize <= result.length);
									GetUtf8BytesForCode(inputDispCodepoint, &result.bytes[outputByteIndex]);
								}
								outputByteIndex += inputDispCharSize;
								break;
							}
						}
					}
					
					
					if (parsedFoundText)
					{
						//NOTE: We assume the closing curly is encoded as 1 byte here. This shouldn't be much of a problem but is maybe technically wrong
						cIndex = closingCurlyIndex + 1;
					}
				}
			}
			
			if (!parsedFoundText)
			{
				if (result.chars != nullptr)
				{
					Assert(outputByteIndex + codepointSize <= result.length);
					MyMemCopy(&result.bytes[outputByteIndex], &input.bytes[cIndex], codepointSize);
				}
				outputByteIndex += codepointSize;
				cIndex += codepointSize;
			}
		}
		
		if (pass == 0)
		{
			if (numItemsParsedOut != nullptr) { *numItemsParsedOut = numItemsParsed; }
			
			if (outputByteIndex == 0 || memArena == nullptr) { return result; }
			
			result.length = outputByteIndex;
			result.bytes = AllocArray(memArena, u8, result.length+1);
		}
		else
		{
			Assert(outputByteIndex == result.length);
			result.chars[result.length] = '\0';
		}
	}
	
	return result;
}

#endif //  _PIG_INPUT_CHARS_H
