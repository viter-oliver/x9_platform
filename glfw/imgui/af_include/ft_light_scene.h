#pragma once
#include "ft_base.h"

namespace auto_future
{
     class AFG_EXPORT ft_light_scene :
     public ft_base
     {
          DEF_STRUCT_WITH_INIT( intl_pt, _pj_pt,
                ( af_vec3,_view_pos ),
                (af_vec3,_center_of_prj),
                (af_vec3,_up),
                (float,_fovy),
                (float,_near),
                (float,_far),
				(af_vec3, _light_direction),
				(af_vec3, _light_position),
				(af_vec3, _light_ambient_clr),
				(af_vec3, _light_diffuse_clr),
				(af_vec3, _light_specular_clr),
                (bool,_test_depth),
                (af_vec4,_bk_clr))
          unsigned int _fboId = { 0 };
          unsigned int _colorTextId = { 0 };
          unsigned int _depthStencilTextId = { 0 };
          void release_resource();
     public:
          ft_light_scene();
          ~ft_light_scene();
          void link();
          void recreate_framebuff();          
          void draw_frames();
          bool handle_mouse();

          af_vec3* get_view_pos()
          {
               return &_pj_pt._view_pos;
          }
          af_vec3* get_center_of_prj()
          {
               return &_pj_pt._center_of_prj;
          }
          af_vec3* get_up()
          {
               return &_pj_pt._up;
          }
          float get_fovy()
          {
               return _pj_pt._fovy;
          }
          float get_near()
          {
               return _pj_pt._near;
          }
          float get_far()
          {
               return _pj_pt._far;
          }
          void set_fovy(float fovy)
          {
               _pj_pt._fovy=fovy;
          }
          void set_far(float ffar)
          {
               _pj_pt._far = ffar;
          }
          void set_near(float fnear)
          {
               _pj_pt._near=fnear;
          }
          af_vec3& get_light_dir(){
               auto plight_dir=&_pj_pt._light_direction;
               return *plight_dir;
          }
          af_vec3& get_light_position() {
               auto plight_pos=&_pj_pt._light_position;
               return *plight_pos;
          }
          af_vec3& get_light_ambient(){
               auto plight_amb=&_pj_pt._light_ambient_clr;
               return *plight_amb;
          }
          af_vec3& get_light_diffuse(){
               auto plight_dif=&_pj_pt._light_diffuse_clr;
               return *plight_dif;
          }
          af_vec3& get_light_specular(){
               auto plight_spec=&_pj_pt._light_specular_clr;
               return *plight_spec;
          }
     };
     REGISTER_CONTROL( ft_light_scene )
}

