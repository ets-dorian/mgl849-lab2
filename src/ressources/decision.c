#include <stddef.h>
#include "decision_engine.h"
#include <stdio.h>
#include <string.h>

int buf_append(char *dst, size_t dst_size, const char *src)
{
    if (!src || src[0] == '\0') return 0;
 
    size_t cur_len = strlen(dst);
    if (cur_len > 0) {
        /* place pour ';' + src + '\0' */
        if (cur_len + 1 + strlen(src) + 1 > dst_size) return 0;
        dst[cur_len] = ';';
        strncpy(dst + cur_len + 1, src, dst_size - cur_len - 2);
    } else {
        strncpy(dst, src, dst_size - 1);
    }
    dst[dst_size - 1] = '\0';
    return 1;
}

int create_alert_cmd(int gas_idx, alarm_t alarm, alarm_t prev_alarm, char *buf, size_t buf_size)
{
    buf[0] = '\0';
 
    if (alarm == prev_alarm) return 0;
 
    const char *suffix;
    switch (alarm) {
    case LOW:    suffix = "L"; break;
    case MEDIUM: suffix = "M"; break;
    case HIGH:   suffix = "H"; break;
    default:     suffix = "";  break;   /* NONE → AG{n}  (reset alarme) */
    }
 
    snprintf(buf, buf_size, "AG%d%s", gas_idx + 1, suffix);
    return 1;
}

int cancel_active(int gas_idx, gas_state_t *st, char *buf, size_t buf_size)
{
    buf[0] = '\0';
 
    switch (st->active_type) {
    case ACTION_AERATION:
        strncpy(buf, "AN", buf_size - 1);
        break;
    case ACTION_VENTILATION:
        strncpy(buf, "VN", buf_size - 1);
        break;
    case ACTION_INJECTION:
        snprintf(buf, buf_size, "AIG%d", gas_idx + 1);
        break;
    default:
        /* Rien d'actif */
        st->active_type  = ACTION_NONE;
        st->active_level = 0;
        return 0;
    }
 
    buf[buf_size - 1] = '\0';
    st->active_type  = ACTION_NONE;
    st->active_level = 0;
    return 1;
}

int choose_action(int gas_idx, alarm_t alarm, gas_state_t *st, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    char cancel_buf[16];
 
    switch (alarm) {
 
    case NONE:
        return cancel_active(gas_idx, st, buf, buf_size);
 
    case LOW:
        if (st->active_type == ACTION_NONE) {
            strncpy(buf, "AL1", buf_size - 1);
            buf[buf_size - 1] = '\0';
            st->active_type  = ACTION_AERATION;
            st->active_level = 1;
            return 1;
        }
        if (st->active_type == ACTION_AERATION && st->active_level == 1) {
            strncpy(buf, "AL2", buf_size - 1);
            buf[buf_size - 1] = '\0';
            st->active_level = 2;
            return 1;
        }
        /* AL2+ ou action plus forte → pas d'escalade inutile */
        return 0;
 
    case MEDIUM:
        if (st->active_type == ACTION_NONE ||
            (st->active_type == ACTION_AERATION && st->active_level < 3)) {
 
            /* Annuler l'aération faible avant de monter */
            if (st->active_type == ACTION_AERATION) {
                cancel_active(gas_idx, st, cancel_buf, sizeof(cancel_buf));
                buf_append(buf, buf_size, cancel_buf);   /* "AN" */
            }
            buf_append(buf, buf_size, "AL3");
            st->active_type  = ACTION_AERATION;
            st->active_level = 3;
            return 1;
        }
        if (st->active_type == ACTION_AERATION && st->active_level == 3) {
            /* Toujours MEDIUM après AL3 → escalade VL1 */
            cancel_active(gas_idx, st, cancel_buf, sizeof(cancel_buf));
            buf_append(buf, buf_size, cancel_buf);       /* "AN" */
            buf_append(buf, buf_size, "VL1");
            st->active_type  = ACTION_VENTILATION;
            st->active_level = 1;
            return 1;
        }
        if (st->active_type == ACTION_INJECTION ||
            (st->active_type == ACTION_VENTILATION && st->active_level >= 2)) {
            /* Retour de HIGH → MEDIUM : annuler l'action haute et revenir à VL1 */
            cancel_active(gas_idx, st, cancel_buf, sizeof(cancel_buf));
            buf_append(buf, buf_size, cancel_buf);
            buf_append(buf, buf_size, "VL1");
            st->active_type  = ACTION_VENTILATION;
            st->active_level = 1;
            return 1;
        }
        return 0;
 
    case HIGH:
        if (st->active_type != ACTION_VENTILATION &&
            st->active_type != ACTION_INJECTION) {
            /* IMPORTANT : cancel AVANT de modifier st */
            cancel_active(gas_idx, st, cancel_buf, sizeof(cancel_buf));
            buf_append(buf, buf_size, cancel_buf);
            buf_append(buf, buf_size, "VL2");
            st->active_type  = ACTION_VENTILATION;
            st->active_level = 2;
            return 1;
        }
        if (st->active_type == ACTION_VENTILATION && st->active_level == 2) {
            /* Toujours HIGH après VL2 → injection ciblée */
            char ig_buf[8];
            snprintf(ig_buf, sizeof(ig_buf), "IG%d", gas_idx + 1);
            buf_append(buf, buf_size, "VN");
            buf_append(buf, buf_size, ig_buf);
            st->active_type  = ACTION_INJECTION;
            st->active_level = 0;
            return 1;
        }
        return 0;
 
    default:
        return 0;
    }
}

