// static Eldbus_Connection *conn = NULL;
// 
// 
// int
// e_auth_init(void)
// {
// //    E_EVENT_AUTH_FPRINT_AVAILABLE = ecore_event_type_new();
// //    E_EVENT_AUTH_FPRINT_STATUS = ecore_event_type_new();
// 
//    conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
//    return 1;
// }
// int
// e_auth_shutdown(void)
// {
//    if (conn) eldbus_connection_unref(conn);
//    conn = NULL;
//    return 1;
// }
// 
// 
// void
// e_auth_fprint_begin(const char *user)
// {
//    Eldbus_Object *obj;
//    Eldbus_Proxy *proxy;
// 
//    printf("FP: e_auth_fprint_begin\n");
//    if (conn)
//      {
//         eina_stringshare_replace(&user_name, user);
//         obj = eldbus_object_get(conn, "net.reactivated.Fprint",
//                                 "/net/reactivated/Fprint/Manager");
//         printf("FP: obj=%p\n", obj);
//         if (obj)
//           {
//              obj_fprint = obj;
// 
//              proxy = eldbus_proxy_get(obj, "net.reactivated.Fprint.Manager");
//              printf("FP: proxy=%p\n", proxy);
//              if (proxy)
//                {
//                   Eldbus_Message *m;
// 
//                   proxy_fprint = proxy;
//                   m = eldbus_proxy_method_call_new(proxy, "GetDefaultDevice");
//                   eldbus_proxy_send(proxy, m, _cb_get_default_device, NULL, -1);
//                }
//           }
//      }
// }
