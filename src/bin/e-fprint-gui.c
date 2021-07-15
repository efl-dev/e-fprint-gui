#ifdef HAVE_CONFIG_H
#include "config.h"

#endif
#include "e-fprint-fprint.c"

/* NOTE: Respecting header order is important for portability.
 * Always put system first, then EFL, then your public header,
 * and finally your private one. */

#include <Ecore_Getopt.h>
#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "eldbus_fprint_device.h"
#include "eldbus_fprint_manager.h"

Eldbus_Connection *conn;
Eldbus_Proxy *new_proxy;
Eldbus_Proxy *new_proxy1;
Eldbus_Pending *p;
Eldbus_Pending *p1;

Evas_Object *ly;
Evas_Object *win;

const char *default_device;

static void enrolled_fingers_cb(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error, Eina_Value *args);
static void _swallow_button();
static void claim_device(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error);


static void
cancel_enroll(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error)
{
   if(error)
   {
      printf("START ENROLL: %s\n", error->message);
      printf("START ENROLL: %s\n", error->error);
   }
}

static void
_response_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
//    Evas_Object *popup = data;
   evas_object_del(obj);
   fprint_device_enroll_stop_call(new_proxy1, cancel_enroll, NULL);
}

static void
_dismiss_hover(void *data, Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Evas_Object *hv = data;

   printf("TEST\n");
   elm_hover_dismiss(hv);
}

static void
_enroll_start_cb(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error)
{
   if(error)
   {
      printf("START ENROLL: %s\n", error->message);
      printf("START ENROLL: %s\n", error->error);
   
//       evas_object_del(data);
      //TODO: Auth fehler anzeigen
   }
}

static void
_popup_enroll_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Evas_Object *popup, *box, *lb, *sep, *ly;

   popup = elm_popup_add(win);
   elm_popup_scrollable_set(popup, EINA_FALSE);
   elm_object_text_set(popup, "Roll or swipe 5 times");
//    elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "block,clicked", _response_cb, popup);
   
   box = elm_box_add(popup);
   evas_object_show(box);
   
   lb = elm_label_add(box);
   elm_object_text_set(lb, "Enroll:");
   evas_object_show(lb);
   elm_box_pack_end(box, lb);
   
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "<bigger>%s</bigger>", data);
   
   lb = elm_label_add(box);
   elm_object_text_set(lb, buf);
   evas_object_show(lb);
   elm_box_pack_end(box, lb);
   
   ly = elm_layout_add(box);
   snprintf(buf, sizeof(buf), "%s/themes/e-fprint-gui.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "enroll");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(ly);
   elm_box_pack_end(box, ly);
         
   Evas_Object *edje_obj = elm_layout_edje_get(ly);

   lb = elm_label_add(box);
   elm_object_text_set(lb, "Press or swipe 5 times"); //TODO: swipe or press auslesen
   evas_object_show(lb);
   elm_box_pack_end(box, lb);
   
   sep = elm_separator_add(box);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, 0.0);
   evas_object_show(sep);
   elm_box_pack_end(box, sep);
   
   lb = elm_label_add(box);
   elm_object_text_set(lb, "<color=green>enroll-stage-passed</color>"); //TODO: swipe or press auslesen
   evas_object_show(lb);
   elm_box_pack_end(box, lb);
   lb = elm_label_add(box);
   elm_object_text_set(lb, "<color=red>enroll-swipe-too-short</color>"); //TODO: swipe or press auslesen
   evas_object_show(lb);
   elm_box_pack_end(box, lb);
   
   sep = elm_separator_add(box);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, 0.0);
   evas_object_show(sep);
   elm_box_pack_end(box, sep);
      

   elm_object_content_set(popup, box);

   
   
   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
   
   //
   fprint_device_enroll_start_call(new_proxy1, _enroll_start_cb, popup, "right-middle-finger");
   //
}



static void
_enroll_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   _popup_enroll_cb(data, NULL);
}

void
fingerprint_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *hv, *bt, *bx, *lb;
   char buf[PATH_MAX];
   const char *layout;
   
   snprintf(buf, sizeof(buf), "<color=white>%s</color>", data);
   
   hv = elm_hover_add(win);

   bx = elm_box_add(win);
   
   
   elm_layout_file_get(ly, NULL, &layout);
   
   if (strcmp(layout, "finger") == 0)
      elm_object_part_content_set(hv, "middle", bx);
   else
      elm_object_part_content_set(hv, "bottom", bx);
      
      
   evas_object_show(bx);
   
   lb = elm_label_add(bx);
   elm_object_text_set(lb, buf);
   evas_object_show(lb);
   elm_box_pack_end(bx, lb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "enroll");
   evas_object_smart_callback_add(bt, "clicked", _dismiss_hover, hv);
   evas_object_smart_callback_add(bt, "clicked", _enroll_cb, data);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "verify");
//    evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_object_text_set(bt, "delete");
//    evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_object_text_set(bt, "delete all");
//    evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, obj);

   evas_object_show(hv);
   
}

static void
_switch_hand(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/themes/e-fprint-gui.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, data);
   
   fprint_device_list_enrolled_fingers_call(new_proxy1, enrolled_fingers_cb, NULL, "simon");
   _swallow_button();
}

static void
_swallow_button()
{
   Evas_Object *swallow_button, *right_list, *left_list, *leftright_list;
   Elm_Object_Item *it;

   // ALL 10 FINGERS

   // LEFT
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Left little finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_left-little-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Left ring finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_left-ring-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Left middle finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_left-middle-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Left index finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_left-index-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Left thumb");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_left-thumb", swallow_button);
   
   // RIGHT
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Right little finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_right-little-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Right ring finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_right-ring-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Right middle finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_right-middle-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Right index finger");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_right-index-finger", swallow_button);
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "Right thumb");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_right-thumb", swallow_button);
   
   // SWITCH LEFT/RIGHT HAND
   leftright_list = elm_list_add(win);
   elm_list_multi_select_set(leftright_list, EINA_FALSE);
   elm_list_select_mode_set(leftright_list, ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT);
   evas_object_size_hint_weight_set(leftright_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      
   elm_list_mode_set(leftright_list, ELM_LIST_EXPAND);
   it = elm_list_item_append(leftright_list, "Left Hand", NULL, NULL, _switch_hand, "left_hand");
//    elm_list_item_selected_set(it, EINA_TRUE);
   elm_list_item_append(leftright_list, "Right Hand", NULL, NULL, _switch_hand, "right_hand");
   evas_object_show(leftright_list);
   elm_object_part_content_set(ly, "swallow_hand_switch", leftright_list);
   
   
   
   // ONE FINGER
   left_list = elm_list_add(win);
   elm_list_multi_select_set(left_list, EINA_FALSE);
   elm_list_select_mode_set(left_list, ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT);
   evas_object_size_hint_weight_set(left_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      
   elm_list_mode_set(left_list, ELM_LIST_EXPAND);
   elm_list_item_append(left_list, "Left little finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(left_list, "Left middle finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(left_list, "Left index finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(left_list, "Left ring finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(left_list, "Left thumb", NULL, NULL, NULL, NULL);
//    evas_object_smart_callback_add(left_list, "clicked", fingerprint_clicked, "Left thumb");
   evas_object_show(left_list);
   elm_object_part_content_set(ly, "swallow_select-finger-right", left_list);
   
   
   
   swallow_button = elm_button_add(win);
   elm_object_style_set(swallow_button, "blank");
   evas_object_smart_callback_add(swallow_button, "clicked", fingerprint_clicked, "FINGER");
   evas_object_show(swallow_button);
   elm_object_part_content_set(ly, "swallow_select-finger", swallow_button);
   
   right_list = elm_list_add(win);
   elm_list_mode_set(right_list, ELM_LIST_EXPAND);
   elm_list_item_append(right_list, "Right little finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(right_list, "Right middle finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(right_list, "Right index finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(right_list, "Right ring finger", NULL, NULL, NULL, NULL);
   elm_list_item_append(right_list, "Right thumb", NULL, NULL, NULL, NULL);
//    evas_object_smart_callback_add(right_list, "clicked", fingerprint_clicked, "Left thumb");
   evas_object_show(right_list);
   elm_object_part_content_set(ly, "swallow_select-finger-left", right_list);
}

static void _select_mode(void *data, Evas_Object *obj, void *event_info)
{  
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/themes/e-fprint-gui.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, data);
   
   fprint_device_list_enrolled_fingers_call(new_proxy1, enrolled_fingers_cb, NULL, "simon");
   _swallow_button();
}

///////////

static void
get_devices(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error, Eina_Value *args)
{
// //    printf("LIST: %i\n", (const char)data);
   if(error)
      printf("ERROR: %s\n", error->error);
   if(error)
   printf("MESSAGE: %s\n", error->message);
}

static void
get_default_device(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error, const char *device)
{
   printf("DEFAULT: %s\n", device);
   if(error)
      printf("ERROR: %s\n", error->error);
   if(error)
   printf("MESSAGE: %s\n", error->message);
}

static void
enrolled_fingers_cb(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error, Eina_Value *args)
{
   Eina_Value *v, array;
   const char *txt;
   unsigned i;
   
   if(error)
   {
      printf("ERROR: %s\n", error->error);
      printf("MESSAGE: %s\n", error->message);
   }else
   {
      eina_value_struct_value_get(args, "arg0", &array);
      for (i = 0; i < eina_value_array_count(&array); i++)
      {
         eina_value_array_get(&array, i, &txt);
         edje_object_signal_emit(ly, "enroll_rif", txt);
         printf("\t%s\n", txt);
         //TODO: Enrolled finger abspreichern, dass es beim Theme wechsel geladen werden kann
      }
      eina_value_flush(&array);
   }
   

}


static void
get_device_proberties(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, const char *value)
{
   char buf[PATH_MAX];
   Evas_Object *lb = data;

   if(value)
   {
      printf("NAME: %s\n", value);
      snprintf(buf, sizeof(buf), "Device Name: <color=white>%s</color>", value);
      elm_object_text_set(lb, buf);
   }
   else
      elm_object_text_set(lb, "NO NAME");
}

static void
get_device_type(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, const char *value)
{
   char buf[PATH_MAX];
   Evas_Object *lb1 = data;

   if(value)
   {
      printf("Type: %s\n", value);
      snprintf(buf, sizeof(buf), "Device Type: <color=white>%s</color>", value);
      elm_object_text_set(lb1, buf);
   }
   else
      elm_object_text_set(lb1, "NO NAME");
}

static void
retry_claim_device(void* data, Evas_Object* o EINA_UNUSED, void* event EINA_UNUSED)
{
   Evas_Object *notify = data;
   
   if(notify)
      evas_object_del(notify);
   
   fprint_device_claim_call(new_proxy1, claim_device, NULL, "simon");
}

static void
claim_device(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error)
{
   if(error){
      printf("ERROR: %s\n", error->error);
      printf("MESSAGE: %s\n", error->message);
      
      Evas_Object *notify, *bx, *bxv, *o;
   
      notify = elm_notify_add(win);
      
      bx = elm_box_add(notify);

      o = elm_label_add(bx);
      elm_object_text_set(o, gettext("Could not claim device<br>Please cancel all other fprint sessions<br>and press retry<br>"));
      evas_object_show(o);
      elm_box_pack_end(bx, o);
      
      bxv = elm_box_add(notify);

      o = elm_button_add(bxv);
      elm_object_text_set(o, gettext("retry"));
      evas_object_smart_callback_add(o, "clicked", retry_claim_device, notify);
      evas_object_show(o);
      elm_box_pack_end(bxv, o);
      
      evas_object_show(bxv);
      
      elm_box_pack_end(bx, bxv);
      
      evas_object_show(bx);
      elm_object_content_set(notify, bx);
      evas_object_show(notify);
   }
}
////////
   
   
EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char** argv EINA_UNUSED)
{
   Evas_Object *box, *lb, *lb1, *h_box, *panel, *hv, *p_box, *sep;
   
   char buf[PATH_MAX];

   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   
   #if ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");
   textdomain(PACKAGE);
   #endif
   //////////////////
   

     
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        fprintf(stderr, "Error: could not get system bus\n");
        return EXIT_FAILURE;
     }
     
   new_proxy = fprint_manager_proxy_get(conn, "net.reactivated.Fprint", "/net/reactivated/Fprint/Manager");

   p1 = fprint_manager_get_default_device_call(new_proxy, get_default_device, NULL);
   
   new_proxy1 = fprint_device_proxy_get(conn, "net.reactivated.Fprint", "/net/reactivated/Fprint/Device/0");
   
   fprint_device_claim_call(new_proxy1, claim_device, NULL, "simon");
   
//    p = fprint_manager_get_devices_call(new_proxy, get_devices, NULL);
   
   fprint_device_list_enrolled_fingers_call(new_proxy1, enrolled_fingers_cb, NULL, "simon");
   
     //////////////////////
   
   // set app informations
   elm_app_name_set("Fingerprint Configuration (e-fprint-gui)");
   elm_app_desktop_entry_set("e-fprint-gui.desktop");
   elm_app_info_set(elm_main, "e-fprint-gui", "");
   
      
   win = elm_win_util_standard_add("e-fprint-gui", "e-fprint-gui Information");
      
   elm_win_title_set(win, gettext("Fingerprint Configuration (e-fprint-gui)"));
   elm_win_autodel_set(win, EINA_TRUE);
      
   box = elm_box_add(win);
   
   evas_object_show(box);

   h_box = elm_box_add(win);
   elm_box_homogeneous_set(h_box, EINA_TRUE);
   elm_box_horizontal_set(h_box, EINA_TRUE);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Choose finger and click on fingerprint to select action");
   evas_object_show(lb);
   elm_box_pack_end(box, lb);
   
   ly = elm_layout_add(h_box);
   snprintf(buf, sizeof(buf), "%s/themes/e-fprint-gui.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "hands");
   evas_object_show(ly);
         
   
   _swallow_button(); // ADD "blank" Buttons for callbacks
   


   elm_box_pack_end(h_box, ly);

   evas_object_show(h_box);
   
   elm_box_pack_end(box, h_box);
      
   h_box = elm_box_add(win);
   elm_box_homogeneous_set(h_box, EINA_TRUE);
   elm_box_horizontal_set(h_box, EINA_TRUE);
   evas_object_size_hint_align_set( h_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

   hv = elm_hoversel_add(h_box);
   elm_object_text_set(hv, "Both hands");
   elm_hoversel_auto_update_set(hv, EINA_TRUE);
   elm_hoversel_hover_parent_set(hv, win);
   elm_hoversel_item_add(hv, "Both hands", NULL, ELM_ICON_NONE, _select_mode, "hands");
   elm_hoversel_item_add(hv, "One hand", NULL, ELM_ICON_NONE, _select_mode, "left_hand");
   elm_hoversel_item_add(hv, "One finger", NULL, ELM_ICON_NONE, _select_mode, "finger");
   evas_object_show(hv);
   elm_box_pack_end(h_box, hv);

   evas_object_show(h_box);

   elm_box_pack_end(box, h_box);

   panel = elm_panel_add(box);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_BOTTOM);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panel);

   
   p_box = elm_box_add(panel);
   elm_box_horizontal_set(p_box, EINA_TRUE);
   evas_object_size_hint_align_set(p_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   
   lb = elm_label_add(panel);
      fprint_device_name_propget(new_proxy1, get_device_proberties, lb); // DEVICE NAME
   evas_object_show(lb);
   elm_box_pack_end(p_box, lb);
   
   sep = elm_separator_add(panel);
   elm_separator_horizontal_set(sep, EINA_FALSE);
   evas_object_show(sep);
   elm_box_pack_end(p_box, sep);
   
   
   lb1 = elm_label_add(panel);
      fprint_device_scan_type_propget(new_proxy1, get_device_type, lb1); // DEVICE TYPE
   evas_object_show(lb1);
   elm_box_pack_end(p_box, lb1);
   
   elm_object_content_set(panel, p_box);
   
   elm_box_pack_end(box, panel);

   evas_object_show(win);
   elm_win_resize_object_add(win, box);

   elm_run();

   return 0;
}
ELM_MAIN()
