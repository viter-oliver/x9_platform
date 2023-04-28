//
// Created by v_f_z on 2022/6/30.
//
/*
car0_show
car0_show
 * */
#include "register_msg_host.h"
#include "adas_can_def.h"
#include "af_bind.h"
#include "af_state_manager.h"
#include "afg.h"
#include "debug_var_set.h"
#include <chrono>
#define vg_print(...)                                                          \
  printf("$$$%s:%d ", __func__, __LINE__);                                     \
  printf(__VA_ARGS__)
using namespace msg_utility;
using namespace auto_future;
using namespace std;
using namespace chrono;
//XXX hud calibration data
namespace hud_cali{
  /**
   |Z
y\ |
  \|____X

adas:
   |z
x\ |
  \|____y

opengl:
   |y
z\ |
  \|____X
*/
  const int level_cnt=21;
  int level{10};
  af_vec3 center_pos[level_cnt]={
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
    {-0.4f,0.9f,7.42f},
  };
  af_vec3& cur_center_pos(){
    return center_pos[level];
  }
  float _fov{4.f};
}
// #define HUD_DEBUG
// UPDATE:controlling of hud controls
namespace hud {
ft_light_scene *pprojector;
ft_4_time_curve_3d *pleft_lane;
ft_4_time_curve_3d *pright_lane;
ft_4_time_wall_3d *pleft_lane_wall;
ft_4_time_wall_3d *pright_lane_wall;
ft_trans *pleft_change_lane;
ft_trans *pright_change_lane;
ft_trans *pmain_tar;
ft_trans *ptar0;
ft_trans *ptar1;
ft_trans *ptar2;
class smoothor {
  static const int uvalue_len = 10;
  float ut_value[uvalue_len];
  int ut_id = {0};
  int buff_cnt = 0;

public:
  enum en_ag { en_linear_3_p, en_linear_5_p, en_linear_5_3, en_linear_cnt };
  void clear() { memset(ut_value, 0, sizeof ut_value); }
  smoothor() { clear(); }

private:
  int ag_id = {en_linear_3_p};
  int ucnt() {
    switch (ag_id) {
    case smoothor::en_linear_3_p:
      return 3;
    case smoothor::en_linear_5_p:
      return 5;
    case smoothor::en_linear_5_3:
      return 5;
    default:
      return 3;
    }
  }

public:
  void set_arg(int aid) {
    aid %= en_linear_cnt;
    ag_id = aid;
    clear();
  }
  float smooth_value(float nf) {
    auto mvcnt = ucnt() - 1;
    for (int ix = 0; ix < mvcnt; ++ix) {
      ut_value[ix] = ut_value[ix + 1];
    }

    switch (ag_id) {
    case smoothor::en_linear_3_p: {

      float ut[3] = {ut_value[0], ut_value[1], nf};
      ut_value[0] = (5 * ut[0] + 2 * ut[1] - ut[2]) / 6;
      ut_value[1] = (ut[0] + ut[1] + ut[2]) / 3;
      ut_value[2] = (2 * ut[1] - ut[0] + 5 * ut[2]) / 6;
      return ut_value[2];
    } break;
    case smoothor::en_linear_5_p: {
      float ut[5] = {ut_value[0], ut_value[1], ut_value[2], ut_value[3], nf};
      ut_value[0] = (3 * ut[0] + 2 * ut[1] + ut[2] - ut[4]) / 5;
      ut_value[1] = (4 * ut[0] + 3 * ut[1] + 2 * ut[2] + ut[3]) / 10;
      ut_value[2] = (ut[0] + ut[1] + ut[2] + ut[3] + ut[4]) / 5;
      ut_value[3] = (ut[1] + 2 * ut[2] + 3 * ut[3] + 4 * ut[4]) / 10;
      ut_value[4] = (ut[2] - ut[0] + 2 * ut[3] + 3 * ut[4]) / 5;
      return ut_value[4];
    } break;
    case smoothor::en_linear_5_3: {
      float ut[5] = {ut_value[0], ut_value[1], ut_value[2], ut_value[3], nf};
      ut_value[0] = (69 * ut[0] + 4 * (ut[1] + ut[3]) - 6 * ut[3] - ut[5]) / 70;
      ut_value[1] =
          (2 * (ut[0] + ut[4]) + 27 * ut[1] + 12 * ut[2] - 8 * ut[3]) / 35;
      ut_value[2] =
          (12 * (ut[1] + ut[3]) - 3 * (ut[0] + ut[4]) + 17 * ut[2]) / 35;
      ut_value[3] =
          (2 * (ut[0] + ut[4]) - 8 * ut[1] + 12 * ut[2] + 27 * ut[3]) / 35;
      ut_value[4] = (4 * (ut[1] + ut[3]) - ut[0] + 6 * ut[2] + 69 * ut[4]) / 70;
      return ut_value[4];
    } break;
    default:
      break;
    }
    return 0.f;
  }
};
smoothor slc0, slc1, slc2, slc3, src0, src1, src2, src3;
#define SMOOTH_GET(m, v) hud::m = hud::s##m.smooth_value(v)
void left_lane_clear() {
  slc0.clear();
  slc1.clear();
  slc2.clear();
  slc3.clear();
}
void right_lane_clear() {
  src0.clear();
  src1.clear();
  src2.clear();
  src3.clear();
}
bool left_lane_valid = false, right_lane_valid = false;
float lc0 = 0.f, lc1 = 0.f, lc2 = 0.f, lc3 = 0.f;
float rc0 = 0.f, rc1 = 0.f, rc2 = 0.f, rc3 = 0.f;

struct adas_tar {
  static float adas_x, adas_z, adas_y;
  float tl_x = 0.f, tl_z = 0.f, tl_y = 0.f;
  ft_trans *phost = nullptr;
  adas_tar &set_tl_x(float fv) {
    tl_x = fv;
    auto xvalue = tl_x + adas_x;
    phost->set_translation_x(xvalue);
    return *this;
  }
  adas_tar &set_tl_y(float fv) {
    tl_y = fv;
    auto yvalue = tl_y + adas_y;
    phost->set_translation_y(yvalue);
    return *this;
  }
  adas_tar &set_tl_z(float fv) {
    tl_z = fv;
    auto zvalue = tl_z + adas_z;
    phost->set_translation_z(zvalue);
    return *this;
  }
  void recal() {
    auto xvalue = tl_x + adas_x;
    phost->set_translation_x(xvalue);
    auto yvalue = tl_y + adas_y;
    phost->set_translation_y(yvalue);
    auto zvalue = tl_z + adas_z;
    phost->set_translation_z(zvalue);
  }
};
float adas_tar::adas_x = 0.f;
float adas_tar::adas_y = 0.f;
float adas_tar::adas_z = 0.f;
adas_tar main_tar, tar0, tar1, tar2;
void set_fovy(float fovy) { pprojector->set_fovy(fovy); }
void set_clip(float ffar, float fnear) {
  pprojector->set_far(ffar);
  pprojector->set_near(fnear);
}
void init_controls() {
  pprojector = (ft_light_scene *)get_aliase_ui_control("show_hud");
  auto& center_pos=*(pprojector->get_center_of_prj());
  center_pos=hud_cali::cur_center_pos();
  pleft_lane = (ft_4_time_curve_3d *)get_aliase_ui_control("show_left_lane");
  pright_lane = (ft_4_time_curve_3d *)get_aliase_ui_control("show_right_lane");
  pleft_lane_wall =
      (ft_4_time_wall_3d *)get_aliase_ui_control("show_left_lane_wall");
  pright_lane_wall =
      (ft_4_time_wall_3d *)get_aliase_ui_control("show_right_lane_wall");
  pleft_change_lane =
      (ft_trans *)get_aliase_ui_control("show_left_change_lane");
  pright_change_lane =
      (ft_trans *)get_aliase_ui_control("show_right_change_lane");
  pmain_tar = (ft_trans *)get_aliase_ui_control("show_main_tar");
  main_tar.phost = pmain_tar;
  ptar0 = (ft_trans *)get_aliase_ui_control("show_tar0");
  tar0.phost = ptar0;
  ptar1 = (ft_trans *)get_aliase_ui_control("show_tar1");
  tar1.phost = ptar1;
  ptar2 = (ft_trans *)get_aliase_ui_control("show_tar2");
  tar2.phost = ptar2;
#ifdef HUD_DEBUG
  pleft_lane->set_visible(true);
  pright_lane->set_visible(true);
  pleft_lane_wall->set_visible(true);
  pright_lane_wall->set_visible(true);
  pmain_tar->set_visible(true);
  ptar0->set_visible(true);
  ptar1->set_visible(true);
  ptar2->set_visible(true);
#endif
}
void calcu_left_lane() {
#if 0
  static int debug_cnt = 0;
  if (debug_cnt == 0) {
    vg_print("lc0=%f,lc1=%f,lc2=%f,lc3=%f\n", lc0, lc1, lc2, lc3);
  }
  debug_cnt++;
  debug_cnt %= 100;
#endif
  pleft_lane->set_coeff(lc0, lc1, lc2, lc3);
  pleft_lane_wall->set_coeff(lc0, lc1, lc2, lc3);
}
void calcu_right_lane() {
  pright_lane->set_coeff(rc0, rc1, rc2, rc3);
  pright_lane_wall->set_coeff(rc0, rc1, rc2, rc3);
}
} // namespace hud

// UPDATE register debug interface
void reg_debug() {
  fifo_debuger::reg_command_handle([](char *pcmd_buff) {
    string str_cmd(pcmd_buff);
    auto eq_pos = str_cmd.find_last_of('=');
    if (eq_pos != -1) {
      string alias_name = str_cmd.substr(0, eq_pos);
      string str_value = str_cmd.substr(eq_pos + 1);
      auto pt_pos = str_value.find('.');
      if (pt_pos != -1) {
        float fvalue = atof(str_value.c_str());
        printf("f alias:%s =%f\n", alias_name.c_str(), fvalue);
        set_property_aliase_value_T(alias_name, fvalue);
      } else {
        int ivalue = atoi(str_value.c_str());
        printf("i alias:%s =%d\n", alias_name.c_str(), ivalue);
        set_property_aliase_value_T(alias_name, ivalue);
      }
    }
    return true;
  });
  fifo_debuger::attach_var_handle("view_x", [](char *pcmd_buff) -> bool {
    float view_x = atof(pcmd_buff);
    hud::pprojector->get_view_pos()->x = view_x;
    return true;
  });
  fifo_debuger::attach_var_handle("view_y", [](char *pcmd_buff) -> bool {
    float view_y = atof(pcmd_buff);
    hud::pprojector->get_view_pos()->y = view_y;
    return true;
  });
  fifo_debuger::attach_var_handle("view_z", [](char *pcmd_buff) -> bool {
    float view_z = atof(pcmd_buff);
    hud::pprojector->get_view_pos()->z = view_z;
    return true;
  });
  fifo_debuger::attach_var_handle("center_x", [](char *pcmd_buff) -> bool {
    float center_x = atof(pcmd_buff);
    hud::pprojector->get_center_of_prj()->x = center_x;
    return true;
  });
  fifo_debuger::attach_var_handle("center_y", [](char *pcmd_buff) -> bool {
    float center_y = atof(pcmd_buff);
    hud::pprojector->get_center_of_prj()->y = center_y;
    return true;
  });
  fifo_debuger::attach_var_handle("center_z", [](char *pcmd_buff) -> bool {
    float center_z = atof(pcmd_buff);
    hud::pprojector->get_center_of_prj()->z = center_z;
    return true;
  });
  fifo_debuger::attach_var_handle("adas_x", [](char *pcmd_buff) -> bool {
    float adas_x = atof(pcmd_buff);
    hud::pleft_lane->set_transx(adas_x);
    hud::pleft_lane_wall->set_transx(adas_x);
    hud::pright_lane->set_transx(adas_x);
    hud::pright_lane_wall->set_transx(adas_x);
    hud::adas_tar::adas_x = adas_x;
    hud::main_tar.recal();
    hud::tar0.recal();
    hud::tar1.recal();
    hud::tar2.recal();
    return true;
  });
  fifo_debuger::attach_var_handle("adas_y", [](char *pcmd_buff) -> bool {
    float adas_y = atof(pcmd_buff);
    hud::pleft_lane->set_transy(adas_y);
    hud::pleft_lane_wall->set_transy(adas_y);
    hud::pright_lane->set_transy(adas_y);
    hud::pright_lane_wall->set_transy(adas_y);
    hud::adas_tar::adas_y = adas_y;
    hud::main_tar.recal();
    hud::tar0.recal();
    hud::tar1.recal();
    hud::tar2.recal();
    return true;
  });
  fifo_debuger::attach_var_handle("adas_z", [](char *pcmd_buff) -> bool {
    float adas_z = atof(pcmd_buff);
    hud::pleft_lane->set_transz(adas_z);
    hud::pleft_lane_wall->set_transz(adas_z);
    hud::pright_lane->set_transz(adas_z);
    hud::pright_lane_wall->set_transz(adas_z);
    hud::adas_tar::adas_z = adas_z;
    hud::main_tar.recal();
    hud::tar0.recal();
    hud::tar1.recal();
    hud::tar2.recal();
    return true;
  });
}

void register_msg_host(msg_utility::msg_host &mh) {
  // UPDATE register adas interface
  {
    mh.register_batch_cmd_handle([](u8 *pbuff, u32 len) -> bool {
      u16 *pcmd = (u16 *)pbuff;
      if (num_in_range(*pcmd, Vehicle_frame_b0, Vehicle_frame_b7)) {
        struct GNU_DEF Vehicle_frame_bx {
          u8 Addition_Vehicle_Number : 2;
          u8 Vehicle_ID : 6;
          u16 Addition_Vehicle_A_PosX : 12; // factor 0.0625
          s16 Addition_Vehicle_A_PosY : 10; // factor 0.0625 -32.f
          u16 Addition_Vehicle_A_Type : 3;
          u16 Vehicle_ID_2 : 6;
          u16 Addition_Vehicle_B_PosX : 12; // factor 0.0625
          s16 Addition_Vehicle_B_PosY : 10; // factor 0.0625 -32.f
          u16 Addition_Vehicle_B_Type : 3;  // no vehicle,car,bus,truck,special
                                            // car
        };
        Vehicle_frame_bx *pcan = (Vehicle_frame_bx *)pbuff + 2;
        float Addition_Vehicle_A_PosX = pcan->Addition_Vehicle_A_PosX * 0.0625f;
        float Addition_Vehicle_A_PosY =
            pcan->Addition_Vehicle_A_PosY * 0.0625f - 32.f;
        float Addition_Vehicle_B_PosX = pcan->Addition_Vehicle_B_PosX * 0.0625f;
        float Addition_Vehicle_B_PosY =
            pcan->Addition_Vehicle_B_PosY * 0.0625f - 32.f;
        // printf("car id:%d posx=%f,posy=%f\n", pcan->Vehicle_ID,
        // Addition_Vehicle_A_PosX, Addition_Vehicle_A_PosY);
        //  sprintf("car id:%d
        //  posx=%f,posy=%f\n",pcan->Vehicle_ID_2,Addition_Vehicle_B_PosX,Addition_Vehicle_B_PosY);

        auto vehicle_id = *pcmd - Vehicle_frame_b0;
        if (vehicle_id == 0) {
          hud::tar0.set_tl_z(Addition_Vehicle_A_PosX)
              .set_tl_x(Addition_Vehicle_A_PosY);
          hud::tar1.set_tl_z(Addition_Vehicle_B_PosX)
              .set_tl_x(Addition_Vehicle_B_PosY);
        } else if (vehicle_id == 1) {
          hud::tar2.set_tl_z(Addition_Vehicle_A_PosX)
              .set_tl_x(Addition_Vehicle_A_PosY);
        }
      } else if (num_in_range(*pcmd, Pedestrian_frame_bb,
                              Pedestrian_frame_bd)) {
        struct GNU_DEF St_Pedestrian_frame_bx {
          u8 Addition_Pedestrian_Number : 2;
          u8 Pedestrian_ID : 6;
          u16 Addition_Pedestrian_A_PosX : 12; // 0.0625 0
          u16 Addition_Pedestrian_A_PosY : 10; // 0.0625 -32
          u16 Target_Pedestrian_A_Type : 3;
          u8 Pedestrian_ID_2 : 6;
          u16 Addition_Pedestrian_B_PosX : 12; // 0.0625 0
          u16 Addition_Pedestrian_B_PosY : 10; // 0.0625 -32
          u16 Target_Pedestrian_B_Type : 3;
        };
      } else if (num_in_range(*pcmd, TSR_frame_B1, TSR_frame_B3)) {
        struct GNU_DEF st_TSR_frame_B {
          u8 Vision_only_Sign_Type;
          u32 reserved : 24;
          u32 CanMsgEnder : 1;
          u32 reserved2 : 7;
        };
      }
      return false;
    });
    mh.register_msg_handle(Vehicle_frame_a1, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_Vehicle_frame_a1 {
        u8 FCW : 1;
        u8 reserved_1 : 1;
        u8 Vehicle_ID : 6;
        u16 Target_Vehicle_PosX : 12; // factor:0.0625
        u16 reseved_2 : 4;
        u8 reseved_2_2 : 8;
        u16 Target_Vehicle_PosY : 10; // factor:0.0625 -31.9375
        u16 reseved_3 : 6;
        u16 Target_Vehicle_Type : 3;
        u16 reseved_4 : 13;
      };
      St_Vehicle_frame_a1 *pcan = (St_Vehicle_frame_a1 *)pbuff;
      float Target_Vehicle_PosX = pcan->Target_Vehicle_PosX * 0.0625f;
      float Target_Vehicle_PosY = pcan->Target_Vehicle_PosY * 0.0625f;
      u16 *pcanid = (u16 *)pbuff;
      static int dcnt = 0;
      if (dcnt == 0) {
        printf("main car(%x): posx:%f,posy:%f___", *pcanid, Target_Vehicle_PosX,
               Target_Vehicle_PosY);
        for (int ix = 0; ix < sizeof(St_Vehicle_frame_a1); ix++) {
          printf("%x|", pbuff[ix]);
        }
        printf("\n");
      }
      dcnt++;
      dcnt %= 100;
      hud::main_tar.set_tl_z(Target_Vehicle_PosX).set_tl_x(Target_Vehicle_PosY);
      return false;
    });
    mh.register_msg_handle(Vehicle_frame_a2, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_Vehicle_frame_a2 {
        u16 CAN_VIS_OBS_TTC_WITH_ACC : 10; // 0.01
        u16 reseved_1 : 6;
        u16 Target_Vehicle_VelX : 12; // 0.0625
        u16 reseved_2 : 4;
        u16 Target_Vehicle_AccelX : 10; // 0.03
        u16 reseved_3 : 6;
        u8 Target_Vehicle_Width;          // 0.05
        u8 Target_Vehicle_Confidence : 6; // 0.02
        u8 reseved_4 : 2;
      };
      St_Vehicle_frame_a2 *pcan = (St_Vehicle_frame_a2 *)pbuff;
      float Target_Vehicle_Width = pcan->Target_Vehicle_Width * 0.05f;
      // printf("main car width=%f\n",Target_Vehicle_Width);
      hud::pmain_tar->set_scale_x(Target_Vehicle_Width)
          .set_scale_y(Target_Vehicle_Width);
      return false;
    });
    mh.register_msg_handle(Vehicle_frame_c, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_Vehicle_frame_c {
        u32 frame_id;
        u8 speed;
      };
      St_Vehicle_frame_c *pcan = (St_Vehicle_frame_c *)pbuff;
      return false;
    });
    mh.register_msg_handle(Pedestrian_frame_a, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_Pedestrian_frame_a {
        u8 Target_Pedestrian_ID : 6;
        u8 reserved_1 : 2;
        u16 Target_Pedestrian_PosX : 12; // 0.0625
        u16 Target_Pedestrian_PosY : 10; // 0.0625 -32.0
        u16 ttc : 10;                    // 0.01
        u16 Target_Pedestrian_VelX : 12; // 0.0625 -127.93
        u16 Target_Pedestrian_VelY : 8;  // 0.0625  -8
        u16 Target_Pedestrian_Type : 3;
        u16 reserved_2 : 1;
      };
      St_Pedestrian_frame_a *pcan = (St_Pedestrian_frame_a *)pbuff;
      return false;
    });
    mh.register_msg_handle(lane_frame_1_l, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_lane_frame_1_l {
        u8 Lane_Type : 4;
        u8 Quality : 2;
        u8 reserved : 2;         // factor 0.0625
        s16 Position;            // 0.00391 -128 128
        u16 Curvature;           // 0.000000976563 -0.032
        u16 CurvatureDerivative; // 0.00000000372529 -0.0001
        u8 WidthMarking;         // 0.01
      };

      St_lane_frame_1_l *pcan = (St_lane_frame_1_l *)pbuff;
      if (pcan->Quality <= adas_value::Low) {
        hud::left_lane_valid = false;
        hud::left_lane_clear();
        return false;
      }
      SMOOTH_GET(lc0, pcan->Position * 0.00390625);
      SMOOTH_GET(lc2, pcan->Curvature * 0.000000976563 - 0.031999023438);
      SMOOTH_GET(lc3, pcan->CurvatureDerivative * 0.00000000372529 -
                          0.00012206658721);
      hud::calcu_left_lane();
#if 0
      static steady_clock::time_point tp_lc0 = steady_clock::now();
      auto nw_tm = steady_clock::now();
      int delta = chrono::duration_cast<chrono::duration<int, std::milli>>(
                      nw_tm - tp_lc0)
                      .count();
      static int dcnt = 0;
      if (dcnt == 0) {
        vg_print("lc0 time span=%d millisec\n", delta);
      }
      dcnt++;
      dcnt %= 10;
      tp_lc0 = nw_tm;
#endif
      return false;
    });
    mh.register_msg_handle(lane_frame_1_h, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_lane_frame_1_h {
        u16 Heading_Angle;    // 0.0000108949 -0.357
        u8 ViewRangeStart;    // 0.5
        u8 ViewRangeEnd;      // 0.5
        u8 LaneCrossing : 1;  // 0-no crossing,1-crossing
        u8 LaneMarkColor : 3; // 0-white,1-yellow
      };
      if (!hud::left_lane_valid) {
        return false;
      }
      St_lane_frame_1_h *pcan = (St_lane_frame_1_h *)pbuff;
      SMOOTH_GET(lc1, pcan->Heading_Angle * 0.0000108949 - 0.357);
      hud::calcu_left_lane();
      return false;
    });
    mh.register_msg_handle(lane_frame_2_l, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_lane_frame_2_l {
        u8 Lane_Type : 4;
        u8 Quality : 2;
        u8 reserved : 2;         // factor 0.0625
        s16 Position;            // 0.00390625 -128
        u16 Curvature;           // 0.000000976563 -0.032
        u16 CurvatureDerivative; // 0.00000000372529 -0.0001
        u8 WidthMarking;
      };
      St_lane_frame_2_l *pcan = (St_lane_frame_2_l *)pbuff;
      if (pcan->Quality <= adas_value::Low) {
        hud::right_lane_valid = false;
        hud::right_lane_clear();
        return false;
      }
      SMOOTH_GET(rc0, pcan->Position * 0.00390625);
      SMOOTH_GET(rc2, pcan->Curvature * 0.000000976563 - 0.031999023438);
      SMOOTH_GET(rc3, pcan->CurvatureDerivative * 0.00000000372529 -
                          0.00012206658721);
      hud::calcu_right_lane();
      return false;
    });
    mh.register_msg_handle(lane_frame_2_h, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_lane_frame_2_h {
        u16 Heading_Angle;    // 0.0000108949
        u8 ViewRangeStart;    // 0.5
        u8 ViewRangeEnd;      // 0.5
        u8 LaneCrossing : 1;  // 0-no crossing,1-crossing
        u8 LaneMarkColor : 3; // 0-white,1-yellow
      };
      if (!hud::right_lane_valid) {
        return false;
      }
      St_lane_frame_2_h *pcan = (St_lane_frame_2_h *)pbuff;
      SMOOTH_GET(rc1, pcan->Heading_Angle * 0.0000108949 - 0.357);
      hud::calcu_right_lane();
      return false;
    });
    mh.register_msg_handle(TSR_frame_A, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_TSR_frame_A {
        u8 Vision_only_Sign_Type;
        u8 reserved[3];
        u8 Vision_only_Sign_Type2;
        u8 reserved2[3];
      };
      St_TSR_frame_A *pcan = (St_TSR_frame_A *)pbuff;
      const char *limit_speed[0xE] = {"10",  "20",  "30",  "40", "50",
                                      "60",  "70",  "80",  "90", "100",
                                      "110", "120", "130", "140"};
      const char *limit_speed2[0xF] = {"5",   "15",  "25",  "35",  "45",
                                       "55",  "65",  "75",  "85",  "95",
                                       "105", "115", "125", "135", "145"};
      if (pcan->Vision_only_Sign_Type >= 100) {
        u8 tsr_id = pcan->Vision_only_Sign_Type - 100;
        set_property_aliase_value("limit_value", limit_speed2[tsr_id]);
      } else {
        u8 tsr_id = pcan->Vision_only_Sign_Type;
        set_property_aliase_value("limit_value", limit_speed[tsr_id]);
      }
      return false;
    });
    // TODO:undefined yet
    mh.register_msg_handle(AEB_WARNING, [&](u8 *pbuff, int len) {
      struct GNU_DEF St_AEB_WARNING {
        u8 SoundType : 3;
        u8 TimeIndicator : 2;
        u8 reserved : 1;
        u8 ZeroSpeed : 1;
        u8 HeadwayValid : 1;
        u8 HeadvayMeasurement : 7; // 0.1
        u8 error : 1;
        u8 LDW_OFF : 1;
        u8 Left_LDW_ON : 1;
        u8 Right_LDW_ON : 1;
        u8 FCW_ON : 1;
        u8 Peds_FCW : 1;
        u8 PedsInDZ : 1;
        u8 TSR_enabled : 1;
      };
      St_AEB_WARNING *pcan = (St_AEB_WARNING *)pbuff;
      return false;
    });
  }
#ifdef SIMULATION_ADAS
  reg_trans_handle("lane_changing", [&](int from, int to) {
    if (from == 1 && to == 2) {
      play_tran_playlist("flash_right_lane_wall", 0);
    }
  });
  reg_trans_handle("flash_right_lane_wall", [&](int from, int to) {
    if (from == 2 && to == 1) {
      play_tran("right_change_lane", 0, 1);
    }
  });
  reg_trans_handle("right_change_lane", [&](int from, int to) {
    static int icnt = 0;
    if (icnt == 3) {
      play_tran_playlist("lane_changing", 0);
      icnt = 0;
    } else {
      play_tran("right_change_lane", 0, 1);
      icnt++;
    }
  });
  play_tran_playlist("lane_changing", 0);
#endif
  // UPDATE: ivi navigation
  {
    mh.register_msg_handle(HUDSERVICE_NEXT_INTERSECTION_NAME_INFO,
                           [&](u8 *pbuff, int len) { return true; });
    mh.register_msg_handle(HUDSERVICE_NEXT_ICON_INFO,
                           [&](u8 *pbuff, int len) { return true; });
    mh.register_msg_handle(HUDSERVICE_NEXT_CROSS_DISTANCE_INFO,
                           [&](u8 *pbuff, int len) { return true; });
    mh.register_msg_handle(HUDSERVICE_TARGET_DISTANCE_INFO,
                           [&](u8 *pbuff, int len) { return true; });
  }
}