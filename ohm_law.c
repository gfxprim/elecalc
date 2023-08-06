//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "ohm_law.h"
#include "libelec.h"

static struct ohm_law_ui {
	gp_widget *r;
	gp_widget *u;
	gp_widget *i;
	gp_widget *p;
	gp_widget *r_unit;
	gp_widget *u_unit;
	gp_widget *i_unit;
	gp_widget *p_unit;
} ohm_law_ui;

static struct elec_val get_r_val(struct ohm_law_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->r)),
		.unit = gp_widget_choice_sel_get(ui->r_unit),
		.type = ELEC_UNIT_RESISTANCE,
	};
}

static struct elec_val get_u_val(struct ohm_law_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->u)),
		.unit = gp_widget_choice_sel_get(ui->u_unit),
		.type = ELEC_UNIT_VOLTAGE,
	};
}

static struct elec_val get_i_val(struct ohm_law_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->i)),
		.unit = gp_widget_choice_sel_get(ui->i_unit),
		.type = ELEC_UNIT_CURRENT,
	};
}

static struct elec_val get_p_val(struct ohm_law_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->p)),
		.unit = gp_widget_choice_sel_get(ui->p_unit),
		.type = ELEC_UNIT_POWER,
	};
}

static void update_r(struct elec_ohm_law ol, struct ohm_law_ui *ui)
{
	size_t r_unit = gp_widget_choice_sel_get(ui->r_unit);

	elec_unit_convert(&ol.r, r_unit);
	gp_widget_tbox_printf(ui->r, "%g", ol.r.val);
}

static void update_u(struct elec_ohm_law ol, struct ohm_law_ui *ui)
{
	size_t u_unit = gp_widget_choice_sel_get(ui->u_unit);

	elec_unit_convert(&ol.u, u_unit);
	gp_widget_tbox_printf(ui->u, "%g", ol.u.val);
}

static void update_i(struct elec_ohm_law ol, struct ohm_law_ui *ui)
{
	size_t i_unit = gp_widget_choice_sel_get(ui->i_unit);

	elec_unit_convert(&ol.i, i_unit);
	gp_widget_tbox_printf(ui->i, "%g", ol.i.val);
}

static void update_p(struct elec_ohm_law ol, struct ohm_law_ui *ui)
{
	size_t p_unit = gp_widget_choice_sel_get(ui->p_unit);

	elec_unit_convert(&ol.p, p_unit);
	gp_widget_tbox_printf(ui->p, "%g", ol.p.val);
}

static void recalc_r_p(struct ohm_law_ui *ui)
{
	struct elec_ohm_law ol = {
		.i = get_i_val(ui),
		.u = get_u_val(ui),
		.r = {.unit = ELEC_UNIT_UNDEF},
		.p = {.unit = ELEC_UNIT_UNDEF},
	};

	elec_ohm_law(&ol);

	update_r(ol, ui);
	update_p(ol, ui);
}

static void recalc_u_p(struct ohm_law_ui *ui)
{
	struct elec_ohm_law ol = {
		.i = get_i_val(ui),
		.u = {.unit = ELEC_UNIT_UNDEF},
		.r = get_r_val(ui),
		.p = {.unit = ELEC_UNIT_UNDEF},
	};

	elec_ohm_law(&ol);

	update_u(ol, ui);
	update_p(ol, ui);
}

static void recalc_i_p(struct ohm_law_ui *ui)
{
	struct elec_ohm_law ol = {
		.i = {.unit = ELEC_UNIT_UNDEF},
		.u = get_u_val(ui),
		.r = get_r_val(ui),
		.p = {.unit = ELEC_UNIT_UNDEF},
	};

	elec_ohm_law(&ol);

	update_i(ol, ui);
	update_p(ol, ui);
}

static void recalc_i_r(struct ohm_law_ui *ui)
{
	struct elec_ohm_law ol = {
		.i = {.unit = ELEC_UNIT_UNDEF},
		.u = get_u_val(ui),
		.r = {.unit = ELEC_UNIT_UNDEF},
		.p = get_p_val(ui),
	};

	elec_ohm_law(&ol);

	update_i(ol, ui);
	update_r(ol, ui);
}

static void recalc_u_r(struct ohm_law_ui *ui)
{
	struct elec_ohm_law ol = {
		.i = get_i_val(ui),
		.u = {.unit = ELEC_UNIT_UNDEF},
		.r = {.unit = ELEC_UNIT_UNDEF},
		.p = get_p_val(ui),
	};

	elec_ohm_law(&ol);

	update_u(ol, ui);
	update_r(ol, ui);
}

static void recalc_i_u(struct ohm_law_ui *ui)
{
	struct elec_ohm_law ol = {
		.i = {.unit = ELEC_UNIT_UNDEF},
		.u = {.unit = ELEC_UNIT_UNDEF},
		.r = get_r_val(ui),
		.p = get_p_val(ui),
	};

	elec_ohm_law(&ol);

	update_u(ol, ui);
	update_i(ol, ui);
}

#define ANY_EQUAL(a, b, c, d) ((a == c && b == d) || (a == d && b == c))

static int tbox_number_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	static gp_widget *edit[2];
	struct ohm_law_ui *ui = ev->self->priv;
	const char *text = gp_widget_tbox_text(ev->self);
	char *end;

	switch (ev->sub_type) {
	case GP_WIDGET_TBOX_POST_FILTER:
		if (!strcmp(text, "-"))
			return 0;

		strtod(text, &end);
		if (*end)
			return 1;
		return 0;
	break;
	case GP_WIDGET_TBOX_EDIT:
		if (edit[0] != ev->self) {
			edit[1] = edit[0];
			edit[0] = ev->self;
		}

		if (ANY_EQUAL(edit[0], edit[1], ui->u, ui->i))
			recalc_r_p(ui);

		if (ANY_EQUAL(edit[0], edit[1], ui->r, ui->i))
			recalc_u_p(ui);

		if (ANY_EQUAL(edit[0], edit[1], ui->r, ui->u))
			recalc_i_p(ui);

		if (ANY_EQUAL(edit[0], edit[1], ui->p, ui->i))
			recalc_u_r(ui);

		if (ANY_EQUAL(edit[0], edit[1], ui->p, ui->u))
			recalc_i_r(ui);

		if (ANY_EQUAL(edit[0], edit[1], ui->p, ui->r))
			recalc_i_u(ui);
	break;
	}

	return 0;
}

static int scale_by_unit(gp_widget_event *ev)
{
	struct ohm_law_ui *ui = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	size_t prev_unit = gp_widget_choice_prev_sel_get(ev->self);
	size_t new_unit = gp_widget_choice_sel_get(ev->self);

	struct elec_ohm_law ol;

	if (ev->self == ui->r_unit) {
		ol.r = get_r_val(ui);
		ol.r.unit = prev_unit;
		elec_unit_convert(&ol.r, new_unit);
		update_r(ol, ui);
	}

	if (ev->self == ui->i_unit) {
		ol.i = get_i_val(ui);
		ol.i.unit = prev_unit;
		elec_unit_convert(&ol.i, new_unit);
		update_i(ol, ui);
	}

	if (ev->self == ui->u_unit) {
		ol.u = get_u_val(ui);
		ol.u.unit = prev_unit;
		elec_unit_convert(&ol.u, new_unit);
		update_u(ol, ui);
	}

	if (ev->self == ui->p_unit) {
		ol.p = get_p_val(ui);
		ol.p.unit = prev_unit;
		elec_unit_convert(&ol.p, new_unit);
		update_p(ol, ui);
	}

	return 0;
}

void ohm_law_init(gp_htable *uids)
{
	ohm_law_ui.r = gp_widget_by_uid(uids, "ohm_r", GP_WIDGET_TBOX);
	ohm_law_ui.u = gp_widget_by_uid(uids, "ohm_u", GP_WIDGET_TBOX);
	ohm_law_ui.i = gp_widget_by_uid(uids, "ohm_i", GP_WIDGET_TBOX);
	ohm_law_ui.p = gp_widget_by_uid(uids, "ohm_p", GP_WIDGET_TBOX);

	gp_widget_on_event_set(ohm_law_ui.r, tbox_number_callback, &ohm_law_ui);
	gp_widget_on_event_set(ohm_law_ui.u, tbox_number_callback, &ohm_law_ui);
	gp_widget_on_event_set(ohm_law_ui.i, tbox_number_callback, &ohm_law_ui);
	gp_widget_on_event_set(ohm_law_ui.p, tbox_number_callback, &ohm_law_ui);

	ohm_law_ui.r_unit = gp_widget_by_cuid(uids, "ohm_r_unit", GP_WIDGET_CLASS_CHOICE);
	ohm_law_ui.u_unit = gp_widget_by_cuid(uids, "ohm_u_unit", GP_WIDGET_CLASS_CHOICE);
	ohm_law_ui.i_unit = gp_widget_by_cuid(uids, "ohm_i_unit", GP_WIDGET_CLASS_CHOICE);
	ohm_law_ui.p_unit = gp_widget_by_cuid(uids, "ohm_p_unit", GP_WIDGET_CLASS_CHOICE);

	gp_widget_on_event_set(ohm_law_ui.r_unit, scale_by_unit, &ohm_law_ui);
	gp_widget_on_event_set(ohm_law_ui.u_unit, scale_by_unit, &ohm_law_ui);
	gp_widget_on_event_set(ohm_law_ui.i_unit, scale_by_unit, &ohm_law_ui);
	gp_widget_on_event_set(ohm_law_ui.p_unit, scale_by_unit, &ohm_law_ui);
}
