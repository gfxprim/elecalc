//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>

#include "libelec.h"
#include "ohm_law.h"

static gp_app_info app_info = {
	.name = "elecalc",
	.desc = "An electrical calculator",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://github.com/gfxprim/elecalc",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2023"},
		{}
	}
};

static struct resistance_ui {
	gp_widget *length;
	gp_widget *unit_length;

	gp_widget *area;
	gp_widget *unit_area;

	gp_widget *diameter;
	gp_widget *unit_diameter;

	gp_widget *material;

	gp_widget *resistance;

	gp_widget *material_name;
	gp_widget *material_comp;
	gp_widget *material_r;
	gp_widget *material_tc;
	gp_widget *material_density;

	gp_widget *res_resistance;
	gp_widget *res_diameter;
	gp_widget *res_area;
	gp_widget *res_mass;
} resistance_ui;

struct ohm_law_ui {
	gp_widget *r;
	gp_widget *v;
	gp_widget *i;
	gp_widget *p;
	gp_widget *r_unit;
	gp_widget *v_unit;
	gp_widget *i_unit;
	gp_widget *p_unit;
} ohm_law_ui;

static struct elec_val get_area_val(struct resistance_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->area)),
		.unit = gp_widget_choice_sel_get(ui->unit_area),
		.type = ELEC_UNIT_AREA,
	};
}

static struct elec_val get_length_val(struct resistance_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->length)),
		.unit = gp_widget_choice_sel_get(ui->unit_length),
		.type = ELEC_UNIT_LENGTH,
	};
}

static struct elec_val get_diameter_val(struct resistance_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->diameter)),
		.unit = gp_widget_choice_sel_get(ui->unit_diameter),
		.type = ELEC_UNIT_LENGTH,
	};
}

static struct elec_val get_resistance_val(struct resistance_ui *ui)
{
	return (struct elec_val) {
		.val = atof(gp_widget_tbox_text(ui->resistance)),
		.unit = ELEC_UNIT_OHM,
		.type = ELEC_UNIT_RESISTANCE,
	};
}

static int recalc_resistance(gp_widget_event *ev)
{
	struct resistance_ui *ui = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (gp_widget_tbox_is_empty(ui->length))
		return 0;

	size_t material = gp_widget_choice_sel_get(ui->material);

	struct elec_val area = get_area_val(ui);
	struct elec_val length = get_length_val(ui);
	struct elec_val res;

	res = elec_resistance_block(&elec_material[material], length, area);

	gp_widget_tbox_printf(ui->resistance, "%g", res.val);

	if (ui->res_resistance) {
		elec_unit_autoscale(&res);
		gp_widget_label_printf(ui->res_resistance, "%g %s",
		                       res.val, elec_unit_name(&res));
	}

	if (ui->res_mass) {
		struct elec_val mass;

		mass = elec_mass_block(&elec_material[material], length, area);

		elec_unit_autoscale(&mass);

		gp_widget_label_printf(ui->res_mass, "%g %s", mass.val, elec_unit_name(&mass));
	}

	if (ui->res_area) {
		elec_unit_autoscale(&area);
		gp_widget_label_printf(ui->res_area, "%g %s",
		                       area.val, elec_unit_name(&area));
	}

	if (ui->res_diameter) {
		struct elec_val diameter = get_diameter_val(ui);
		elec_unit_autoscale(&diameter);

		gp_widget_label_printf(ui->res_diameter, "%g %s",
		                       diameter.val, elec_unit_name(&diameter));
	}

	return 0;
}

static void recalc_length(struct resistance_ui *ui)
{
	size_t material = gp_widget_choice_sel_get(ui->material);
	size_t unit_length = gp_widget_choice_sel_get(ui->unit_length);
	struct elec_val area = get_area_val(ui);
	struct elec_val resistance = get_resistance_val(ui);
	struct elec_val length;

	length = elec_length_block(&elec_material[material], resistance, area);

	if (ui->length) {
		elec_unit_convert(&length, unit_length);
		gp_widget_label_printf(ui->length, "%g", length.val);
	}
}

static void recalc_diameter(struct resistance_ui *ui)
{
	size_t unit_diameter = gp_widget_choice_sel_get(ui->unit_diameter);
	struct elec_val value = get_area_val(ui);

	elec_circle_diameter(&value, unit_diameter);

	gp_widget_tbox_printf(ui->diameter, "%g", value.val);
}

static void recalc_area(struct resistance_ui *ui)
{
	struct elec_val value = get_diameter_val(ui);
	size_t unit_area = gp_widget_choice_sel_get(ui->unit_area);

	elec_circle_area(&value, unit_area);

	gp_widget_tbox_printf(ui->area, "%g", value.val);
}

static int tbox_number_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	struct resistance_ui *ui = ev->self->priv;
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
		if (ev->self == ui->diameter)
			recalc_area(ui);

		if (ev->self == ui->area)
			recalc_diameter(ui);

		if (ev->self != ui->resistance)
			recalc_resistance(ev);
		else
			recalc_length(ui);
	break;
	}

	return 0;
}

static void update_material_info(gp_widget *self, struct resistance_ui *ui)
{
	size_t idx = gp_widget_choice_sel_get(self);
	struct elec_material *res = &elec_material[idx];

	gp_widget_label_set(ui->material_name, res->name);
	gp_widget_label_set(ui->material_comp, res->composition);
	gp_widget_label_printf(ui->material_r, "%g \u03a9\u00b7m", res->ro);
	gp_widget_label_printf(ui->material_tc, "%g \u03a9 1/K", res->tc);
	gp_widget_label_printf(ui->material_density, "%g kg/m\u00b3 ", res->density);
}

int material_callback(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	update_material_info(ev->self, ev->self->priv);
	recalc_resistance(ev);

	return 0;
}

const gp_widget_choice_desc units_len_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_units_length,
		.memb_cnt = ELEC_UNIT_LENGTH_CNT,
		.memb_size = sizeof(struct elec_units),
		.memb_off = offsetof(struct elec_units, name),
	}
};

const gp_widget_choice_desc units_area_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_units_area,
		.memb_cnt = ELEC_UNIT_AREA_CNT,
		.memb_size = sizeof(struct elec_units),
		.memb_off = offsetof(struct elec_units, name),
	}
};

const gp_widget_choice_desc units_material_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_material,
		.memb_cnt = ELEC_RESISTIVITY_CNT,
		.memb_size = sizeof(struct elec_material),
		.memb_off = offsetof(struct elec_material, name),
	}
};

const gp_widget_choice_desc units_resistance_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_units_resistance,
		.memb_cnt = ELEC_UNIT_RESISTANCE_CNT,
		.memb_size = sizeof(struct elec_units),
		.memb_off = offsetof(struct elec_units, name),
	}
};

const gp_widget_choice_desc units_voltage_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_units_voltage,
		.memb_cnt = ELEC_UNIT_VOLTAGE_CNT,
		.memb_size = sizeof(struct elec_units),
		.memb_off = offsetof(struct elec_units, name),
	}
};

const gp_widget_choice_desc units_current_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_units_current,
		.memb_cnt = ELEC_UNIT_CURRENT_CNT,
		.memb_size = sizeof(struct elec_units),
		.memb_off = offsetof(struct elec_units, name),
	}
};

const gp_widget_choice_desc units_power_desc = {
	.ops = &gp_widget_choice_arr_ops,
	.arr = &(gp_widget_choice_arr) {
		.ptr = elec_units_power,
		.memb_cnt = ELEC_UNIT_POWER_CNT,
		.memb_size = sizeof(struct elec_units),
		.memb_off = offsetof(struct elec_units, name),
	}
};

int main(int argc, char *argv[])
{
	gp_htable *uids;
	gp_widget *layout = gp_app_layout_load("elecalc", &uids);

	gp_app_info_set(&app_info);

	ohm_law_init(uids);

	resistance_ui.resistance = gp_widget_by_uid(uids, "resistance", GP_WIDGET_TBOX);
	resistance_ui.length = gp_widget_by_uid(uids, "length", GP_WIDGET_TBOX);
	resistance_ui.diameter = gp_widget_by_uid(uids, "diameter", GP_WIDGET_TBOX);
	resistance_ui.area = gp_widget_by_uid(uids, "area", GP_WIDGET_TBOX);

	gp_widget_on_event_set(resistance_ui.resistance, tbox_number_callback, &resistance_ui);
	gp_widget_on_event_set(resistance_ui.length, tbox_number_callback, &resistance_ui);
	gp_widget_on_event_set(resistance_ui.diameter, tbox_number_callback, &resistance_ui);
	gp_widget_on_event_set(resistance_ui.area, tbox_number_callback, &resistance_ui);

	resistance_ui.unit_length = gp_widget_by_cuid(uids, "unit_length", GP_WIDGET_CLASS_CHOICE);
	resistance_ui.unit_area = gp_widget_by_cuid(uids, "unit_area", GP_WIDGET_CLASS_CHOICE);
	resistance_ui.unit_diameter = gp_widget_by_cuid(uids, "unit_diameter", GP_WIDGET_CLASS_CHOICE);
	resistance_ui.material = gp_widget_by_cuid(uids, "material", GP_WIDGET_CLASS_CHOICE);

	gp_widget_on_event_set(resistance_ui.material, material_callback, &resistance_ui);

	gp_widget_on_event_set(resistance_ui.unit_area, recalc_resistance, &resistance_ui);
	gp_widget_on_event_set(resistance_ui.unit_length, recalc_resistance, &resistance_ui);

	resistance_ui.material_name = gp_widget_by_uid(uids, "material_name", GP_WIDGET_LABEL);
	resistance_ui.material_comp = gp_widget_by_uid(uids, "material_comp", GP_WIDGET_LABEL);
	resistance_ui.material_r = gp_widget_by_uid(uids, "material_r", GP_WIDGET_LABEL);
	resistance_ui.material_tc = gp_widget_by_uid(uids, "material_tc", GP_WIDGET_LABEL);
	resistance_ui.material_density = gp_widget_by_uid(uids, "material_density", GP_WIDGET_LABEL);

	resistance_ui.res_resistance = gp_widget_by_uid(uids, "res_resistance", GP_WIDGET_LABEL);
	resistance_ui.res_diameter = gp_widget_by_uid(uids, "res_diameter", GP_WIDGET_LABEL);
	resistance_ui.res_area = gp_widget_by_uid(uids, "res_area", GP_WIDGET_LABEL);
	resistance_ui.res_mass = gp_widget_by_uid(uids, "res_mass", GP_WIDGET_LABEL);

	update_material_info(resistance_ui.material, &resistance_ui);

	gp_widgets_main_loop(layout, "elecalc", NULL, argc, argv);

	return 0;
}
