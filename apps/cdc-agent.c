/*
 * Copyright 2016 Vladimir Korol <vovabox@mail.ru>
 *
 * This file is part of ikbus-apps.
 *
 * ikbus-apps is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ikbus-apps is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ikbus-apps. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <gio/gio.h>
#include <playerctl.h>
#include "ikbuscdc.h"

#define CONFIGDIR "/etc"
#define CONFIG_NAME "cdc.conf"

#define MAGAZINE_SIZE 6


static GKeyFile *cdc_conf;
static GDBusProxy *session;
static GMainLoop *loop;

static const gchar * const supported_options[] = {
    /* [Magazine] */
    "cd1",
    "cd2",
    "cd3",
    "cd4",
    "cd5",
    "cd6",
};

enum {
  PLAY,
  STOP,
  METADATA,
  LAST_SIGNAL
};

typedef struct cd {
    guint number;
    PlayerctlPlayer *mpris;
    gboolean active;
    gchar *mpris_name;
    gulong signal_id[LAST_SIGNAL];
} cd_t;

static struct {
    IKBusCdc *cdc;
    cd_t magazine[MAGAZINE_SIZE];
    guint num_of_cds;
    cd_t *current_cd;
} cd_changer = {
    .cdc = NULL,
    .magazine = {
        {1, NULL, FALSE, NULL, {0}},
        {2, NULL, FALSE, NULL, {0}},
        {3, NULL, FALSE, NULL, {0}},
        {4, NULL, FALSE, NULL, {0}},
        {5, NULL, FALSE, NULL, {0}},
        {6, NULL, FALSE, NULL, {0}},
    },
    .num_of_cds = 0,
    .current_cd = NULL,
};

static GKeyFile *load_config(const char *file)
{
    GError *err = NULL;
    GKeyFile *keyfile;

    keyfile = g_key_file_new();

    g_key_file_set_list_separator(keyfile, ',');

    if (!g_key_file_load_from_file(keyfile, file, 0, &err)) {
        if (!g_error_matches(err, G_FILE_ERROR, G_FILE_ERROR_NOENT))
            g_critical("Parsing %s failed: %s", file, err->message);
        g_error_free(err);
        g_key_file_free(keyfile);
        return NULL;
    }

    return keyfile;
}

static void check_config(GKeyFile *config)
{
    const gchar *valid_groups[] = { "Magazine", NULL };
    gchar **keys;
    gint i;

    if (!config)
        return;

    keys = g_key_file_get_groups(config, NULL);

    for (i = 0; keys != NULL && keys[i] != NULL; i++) {
        const gchar **group;
        gboolean match = FALSE;

        for (group = valid_groups; *group; group++) {
            if (g_str_equal(keys[i], *group)) {
                match = TRUE;
                break;
            }
        }

        if (!match)
            g_warning("Unknown group %s in cdc.conf", keys[i]);
    }

    g_strfreev(keys);

    keys = g_key_file_get_keys(config, "Magazine", NULL, NULL);

    for (i = 0; keys != NULL && keys[i] != NULL; i++) {
        gboolean found;
        guint j;

        found = FALSE;
        for (j = 0; j < G_N_ELEMENTS(supported_options); j++) {
            if (g_str_equal(keys[i], supported_options[j])) {
                found = TRUE;
                break;
            }
        }

        if (!found)
            g_warning("Unknown key %s in %s", keys[i], CONFIG_NAME);
    }

    g_strfreev(keys);
}

/* Get players from config file */
static void parse_config(GKeyFile *config)
{
    GError *err = NULL;
    gchar *str;
    guint i;

    if (!config)
        return;

    for (i = 0; i < MAGAZINE_SIZE; i++) {
        str = g_key_file_get_string(config, "Magazine", supported_options[i], &err);
        if (err) {
            g_print("%s", err->message);
            g_clear_error(&err);
        } else {
            cd_changer.magazine[i].mpris_name = str;
            cd_changer.num_of_cds++;
        }

    }
}

/*
 PLAYBACK
 */

void mpris_metadata(PlayerctlPlayer *player, GVariant *metadata, gpointer data)
{
    GValue str = G_VALUE_INIT;
    gchar *name;
    gint tracknum = -1;
    gint cdnum = *((gint *) data);

    if (cdnum != cd_changer.current_cd->number)
        return;

    tracknum = g_ascii_strtod (playerctl_player_print_metadata_prop(player, "xesam:trackNumber", NULL), NULL);
    ikbus_cdc_set_track(cd_changer.cdc, tracknum);
    g_usleep (100000);
    ikbus_cdc_sync_output(cd_changer.cdc, NULL);
}

void mpris_play(PlayerctlPlayer *player, gpointer data)
{

}

void mpris_stop(PlayerctlPlayer *player, gpointer data)
{

}

static cd_t *find_cd_by_player_name(const gchar *player_name)
{
    guint i;
    cd_t *ret = NULL;

    if (player_name == NULL)
        return ret;

    for (i = 0; i < MAGAZINE_SIZE; i++) {
        if (g_strcmp0(cd_changer.magazine[i].mpris_name, player_name) == 0) {
            ret = &cd_changer.magazine[i];
            break;
        }
    }

    return ret;
}

static void attach_player_to_cd(gchar* player_name, cd_t *cd)
{
    PlayerctlPlayer *mpris = NULL;
    GError *error = NULL;

    if ((player_name == NULL) || (cd == NULL))
        return;

    if (cd->active == TRUE) {
        g_warning("attach_player_to_cd: Double attach cd%d\n", cd->number);
        return;
    }

    mpris = playerctl_player_new(player_name, &error);
    if (mpris == NULL) {
        g_warning("add_player: %s\n", error->message);
         return;
    }

    cd->mpris = mpris;
    g_signal_connect(G_OBJECT (mpris), "play", G_CALLBACK(mpris_play), &cd->number);
    g_signal_connect(G_OBJECT (mpris), "stop", G_CALLBACK(mpris_stop), &cd->number);
    g_signal_connect(G_OBJECT (mpris), "metadata", G_CALLBACK(mpris_metadata), &cd->number);
    cd->active = TRUE;
    ikbus_cdc_insert_cd(cd_changer.cdc, cd->number);
    if (cd_changer.current_cd == NULL) {
        cd_changer.current_cd = cd;
        ikbus_cdc_set_cd(cd_changer.cdc, cd_changer.current_cd->number);
        ikbus_cdc_set_error (cd_changer.cdc, 0);
    }
    g_print("Attach %s to cd%d\n", player_name, cd->number);
}

static void deatach_player(cd_t *cd)
{
    if (cd == NULL)
        return;

    if (cd->active = TRUE) {
        guint i;
        g_clear_object(&cd->mpris);
        cd->active = FALSE;
        ikbus_cdc_remove_cd(cd_changer.cdc, cd->number);
        g_print("Detach cd%d\n", cd->number);
        if (cd->number != cd_changer.current_cd->number)
            return;

        for (i = 0; (i < MAGAZINE_SIZE) && (cd_changer.magazine[i].active != TRUE); i++);
        if (i < MAGAZINE_SIZE) {
            cd_changer.current_cd = &cd_changer.magazine[i];
            ikbus_cdc_set_cd(cd_changer.cdc, cd_changer.current_cd->number);
            
        }
        else {
            /* No discs in magazine */
            /* set error "NO DISC" */
            cd_changer.current_cd = NULL;
            ikbus_cdc_set_cd(cd_changer.cdc, 0);
        }
    }
}

void ikbus_play(IKBusCdc *cdc, gpointer data)
{
    if (cd_changer.current_cd != NULL)
        playerctl_player_play(cd_changer.current_cd->mpris, NULL);
}

void ikbus_stop(IKBusCdc *cdc, gpointer data)
{
    if (cd_changer.current_cd != NULL)
        playerctl_player_pause(cd_changer.current_cd->mpris, NULL);
}

void ikbus_next(IKBusCdc *cdc, gpointer data)
{
    if (cd_changer.current_cd != NULL)
        playerctl_player_next(cd_changer.current_cd->mpris, NULL);
}

void ikbus_previous(IKBusCdc *cdc, gpointer data)
{
    if (cd_changer.current_cd != NULL)
        playerctl_player_previous(cd_changer.current_cd->mpris, NULL);
}

void ikbus_ch_disc(IKBusCdc *cdc,gpointer arg, gpointer data)
{
    PlayerctlPlayer *mpris = cd_changer.current_cd->mpris;
    guint cdnum = ikbus_cdc_get_cmd_arg(cd_changer.cdc);

    if (cd_changer.current_cd->number == cdnum) {
        playerctl_player_play_pause(cd_changer.current_cd->mpris, NULL);
        return;
    }

    if (cd_changer.magazine[cdnum -1].active == TRUE) {
        playerctl_player_pause(cd_changer.current_cd->mpris, NULL);
        cd_changer.current_cd = &cd_changer.magazine[cdnum -1];
        playerctl_player_play(cd_changer.current_cd->mpris, NULL);
        ikbus_cdc_set_cd (cd_changer.cdc, cdnum);
        ikbus_cdc_sync_output(cd_changer.cdc, NULL);
    }

}

static gboolean player_have_mpris(const gchar* player_name)
{
    GVariant *reply;
    GVariant *reply_child;
    const gchar **names;
    gchar *player_bus_name;
    guint i;
    gsize reply_count;
    gboolean ret = FALSE;

    if (session == NULL)
        return ret;

    reply = g_dbus_proxy_call_sync(session, "ListNames", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);
    if (reply == NULL)
        return ret;

    player_bus_name = g_strjoin(".", "org.mpris.MediaPlayer2", player_name, NULL);
    reply_child = g_variant_get_child_value(reply, 0);
    names = g_variant_get_strv(reply_child, &reply_count);
    for (i = 0; i < reply_count; i += 1) {
        if (g_strcmp0(names[i], player_bus_name) == 0) {
            ret = TRUE;
            break;
        }
    }
    g_variant_unref(reply);
    g_variant_unref(reply_child);
    g_free(names);
    g_free(player_bus_name);

    return ret;
}

static void dbus_signal (GDBusProxy *proxy,
           gchar      *sender_name,
           gchar      *signal_name,
           GVariant   *parameters,
           gpointer    user_data)
{
  gchar *bus_name, *old, *new, *player_name;
  gchar **split_bus_name;
  cd_t *cd;

  if ((g_strcmp0(signal_name, "NameOwnerChanged") != 0) || (!g_variant_is_of_type(parameters ,G_VARIANT_TYPE ("(sss)"))))
      return;

  g_variant_get(parameters, "(sss)", &bus_name, &old, &new);
  if (!g_str_has_prefix(bus_name, "org.mpris.MediaPlayer2."))
      return;

  split_bus_name = g_strsplit(bus_name, ".", 4);
  player_name = g_strdup(split_bus_name[3]);
  cd = find_cd_by_player_name(player_name);
  if (cd == NULL)
      goto free;

  if (*new == '\0') {
      /*remove player*/
      deatach_player(cd);
      
  }
  else {
      /*add player*/
      attach_player_to_cd(player_name, cd);
  }
  ikbus_cdc_sync_output(cd_changer.cdc, NULL);

  //ikbus_cdc_sync_output(cd_changer.cdc, NULL);
free:
  g_free (bus_name); 
  g_free (old); 
  g_free (new);
  g_free (player_name);
}

int main(int argc, char **argv)
{
    GError *error = NULL;
    gchar *conf_file;
    guint i;


    /* Load configure */
    conf_file = g_build_filename(g_get_home_dir(),".config", "cdc", CONFIG_NAME, NULL);
    if (g_file_test(conf_file, G_FILE_TEST_EXISTS)) {
        cdc_conf = load_config(conf_file);
    }
    else {
        g_free(conf_file);
        conf_file = g_build_filename(CONFIGDIR, CONFIG_NAME, NULL);
        if (g_file_test(conf_file, G_FILE_TEST_EXISTS)) {
            cdc_conf = load_config(conf_file);
        }
        else {
            g_critical("Could not load config file.\n");
            return -1;
        }
    }
    parse_config(cdc_conf);

    if (cd_changer.num_of_cds < 1) {
        g_critical("Could not find CD in %s\n", conf_file);
        g_free(conf_file);
        return -1;
    }
    g_free(conf_file);

    /* Init CDC device connected to I/K-bus */
    cd_changer.cdc = ikbus_cdc_new("ibus0", &error);
    if (cd_changer.cdc == NULL) {
        g_critical("IKBus: %s\n", error->message);
        return -1;
    }

    /*Connect to org.freedesktop.DBus*/
    session = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            NULL,
                                            "org.freedesktop.DBus",
                                            "/org/freedesktop/DBus",
                                            "org.freedesktop.DBus",
                                            NULL,
                                            &error);
    if (session == NULL) {
        g_critical("Dbus: %s\n", error->message);
        return -1;
    }
    g_signal_connect (session, "g-signal", G_CALLBACK (dbus_signal), NULL);

    /* Attach MPRIS2 interfaces to control */
    ikbus_cdc_set_error (cd_changer.cdc, CDC_ERR_NO_DISCS);
    for (i = 0; i < MAGAZINE_SIZE; i++) {
        if (cd_changer.magazine[i].mpris_name != NULL) {
            cd_changer.magazine[i].mpris = playerctl_player_new(cd_changer.magazine[i].mpris_name, &error);
            if (cd_changer.magazine[i].mpris == NULL) {
                g_warning("playerctl: %s\n", error->message);
                continue;
            }
            if (player_have_mpris(cd_changer.magazine[i].mpris_name) == TRUE) {
                attach_player_to_cd(cd_changer.magazine[i].mpris_name, &cd_changer.magazine[i]);
            }
        }
    }

    /* Signals from I/K-bus from automotive ECU */
    g_signal_connect(G_OBJECT (cd_changer.cdc), "play", G_CALLBACK (ikbus_play), NULL);
    g_signal_connect(G_OBJECT (cd_changer.cdc), "stop", G_CALLBACK (ikbus_stop), NULL);
    g_signal_connect(G_OBJECT (cd_changer.cdc), "next", G_CALLBACK (ikbus_next), NULL);
    g_signal_connect(G_OBJECT (cd_changer.cdc), "previous", G_CALLBACK (ikbus_previous), NULL);
    g_signal_connect(G_OBJECT (cd_changer.cdc), "change-disc", G_CALLBACK (ikbus_ch_disc), NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
