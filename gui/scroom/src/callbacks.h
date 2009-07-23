#include <gtk/gtk.h>


#ifdef __cplusplus
extern "C"
{
#endif

  void
  on_scroom_hide                         (GtkWidget       *widget,
                                          gpointer         user_data);

  gboolean
  on_scroom_destroy_event                (GtkWidget       *widget,
                                          GdkEvent        *event,
                                          gpointer         user_data);

  void
  on_new_activate                        (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_open_activate                       (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_save_activate                       (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_save_as_activate                    (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_quit_activate                       (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_cut_activate                        (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_copy_activate                       (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_paste_activate                      (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_delete_activate                     (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  void
  on_about_activate                      (GtkMenuItem     *menuitem,
                                          gpointer         user_data);

  gboolean
  on_drawingarea_expose_event            (GtkWidget       *widget,
                                          GdkEventExpose  *event,
                                          gpointer         user_data);

  gboolean
  on_drawingarea_configure_event         (GtkWidget       *widget,
                                          GdkEventConfigure *event,
                                          gpointer         user_data);

  gboolean
  on_idle                                (gpointer         user_data);

  void
  on_scroom_bootstrap                    (GtkWidget       *scroom);
  

#ifdef __cplusplus
}
#endif

