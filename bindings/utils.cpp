#include "utils.h"
#include <core/map.h>
#include <core/ustring.h>
#include <core/string_db.h>
#include <core/class_db.h>
#include "../angelscript.h"

namespace asb {

#ifdef TOOLS_ENABLED

String get_script_type_name(const PropertyInfo& pi, bool novoid) {
	String type = Variant::get_type_name(pi.type);
	type = (type == "Object") ? (String)pi.class_name : type;
	if (type == Variant::get_type_name(Variant::NIL)) {
		type = novoid ? "Variant" : "void";
	}
	type = ClassDB::class_exists(type) ? type + "@" : type;
	type = type.empty() ? "Variant" : type;
	return type;
}

String get_binding_script_content() {
	static String script_bindings;
	if (!script_bindings.empty()) {
		return script_bindings;
	}
	static const String FILE_TEMPLATE = "// THIS FILE IS GENERATED BY TOO DO NOT MODIFY\n"
										"namespace godot {\n"
										"{classes}\n"
										"}";
	static const String OBJECT_TEMPLATE = R"(
	{class_doc}
	class {class}{inherits} {
		{methods}

		protected void _make_instance() { @ptr = bindings::instance_class(bindings::id_{class}); }
		{extention}
	})";

	static const String REFERENCE_TEMPLATE = R"(
	{class_doc}
	class {class}{inherits} {
		{methods}

		protected void _make_instance() { @ptr = (ref = bindings::instance_class(bindings::id_{class})).ptr(); }
		{extention}
	})";

	static const String METHOD_TEMPLATE = R"(
		{method_doc}
		{permission}{return_type} {method_name}({params_list}) {qualifier}{
			{return}ptr.godot_icall(bindings::id_{class}_{method_name_raw}{params});
		})";
	static const String METHOD_RO_TEMPLATE = R"(
		{method_doc}
		{permission}{return_type} {method_name}({params_list}) {qualifier}{
			{tmp_type} ret = ptr.godot_icall(bindings::id_{class}_{method_name_raw}{params});
			{return} ret;
		})";
	static const String VMETHOD_TEMPLATE = R"(
		{method_doc}
		{permission}{return_type} {method_name}({params_list}) {qualifier}{
			{return}
		})";
	static const String OBJECT_EXT_TEMPLATE = R"(
		Variant opImplConv() const { return @ptr; }
		void opAssign(const Variant &in ptr) { @this.ptr = ptr; }

		Object() { _make_instance(); }
		void free() { ptr.free(); }

		protected bindings::Object@ ptr;
	)";
	static const String REFERENCE_EXT_TEMPLATE = R"(
		Variant opImplConv() const { return ref; }
		void opAssign(const Variant &in ref) { @ptr = (this.ref = ref).ptr(); }

		protected REF ref;
	)";

	static Map<String, String> vmethod_return_exp;
	vmethod_return_exp["int"] = "return 0;";
	vmethod_return_exp["void"] = "";
	vmethod_return_exp["float"] = "return 0;";
	vmethod_return_exp["bool"] = "return false;";

	List<String> keywords;
	AngelScriptLanguage::get_singletion()->get_reserved_words(&keywords);

	String classes = "";
	const StringName ReferenceName = StringName("Reference");
	const StringName ObjectName = StringName("Object");

	const StringName *class_key = NULL;
	while (class_key = ClassDB::classes.next(class_key)) {
		ClassDB::ClassInfo * cls = ClassDB::classes.getptr(*class_key);
		// FIXME: How about this class ?
		if (*class_key == "WeakRef") {
			continue;
		}

		Dictionary class_info;
		class_info["class"] = (String)(*class_key);
		String inherits = cls->inherits;
		class_info["inherits"] = inherits.empty() ? "" : String(" : ") + inherits;
		class_info["class_doc"] = "";
		class_info["extention"] = "";
		if ((*class_key) == "Object") {
			class_info["extention"] = OBJECT_EXT_TEMPLATE;
		} else if ((*class_key) == "Reference") {
			class_info["extention"] = REFERENCE_EXT_TEMPLATE;
		}

		// Methods
		{
			String methods = "";
			List<MethodInfo> method_list;
			ClassDB::get_method_list(*class_key, &method_list, true, false);
			for (List<MethodInfo>::Element * E = method_list.front(); E; E = E->next()) {

				const MethodInfo& mi = E->get();

				if (*(class_key) == ObjectName && mi.name == "free") {
					continue;
				}

				bool has_ret_val = false;
				MethodBind **mb = cls->method_map.getptr(StringName(mi.name));
				has_ret_val = (mb && *mb) && (*mb)->has_return();

				Dictionary method_info;
				method_info["class"] = (String)(*class_key);
				method_info["method_name_raw"] = mi.name;

				String method_name = mi.name;
				if (keywords.find(method_name) != NULL) {
					method_name = method_name.capitalize();
				}
				method_info["method_name"] = method_name;

				String ret_type = get_script_type_name(mi.return_val, has_ret_val);
				method_info["return_type"] = ret_type;
				method_info["tmp_type"] = ret_type.replace("@", "");
				method_info["return"] = ret_type == "void" ? "" : "return ";
				method_info["qualifier"] = mi.flags & MethodFlags::METHOD_FLAG_CONST ? "const " : "";
				method_info["permission"] = mi.name.begins_with("_") ? "protected " : "";
				method_info["method_doc"] = "";
				// Parameters
				{
					String params_list = "";
					String params = "";
					for (int i = 0; i < mi.arguments.size(); i++) {
						const PropertyInfo& pi = mi.arguments[i];
						String arg_type = Variant::get_type_name(pi.type);
						arg_type = (arg_type == Variant::get_type_name(Variant::NIL)) ? "Variant" : arg_type;
						if (arg_type == "Object") {
							String type = pi.class_name;
							arg_type = type.empty() ? "Object@" : (String)pi.class_name + "@";
						} else {
							if (arg_type == "bool" || arg_type == "int" || arg_type == "float") {
								arg_type = arg_type;
							} else {
								arg_type = String("const ") + arg_type + " &in";
							}
						}

						String arg_name = pi.name;
						if (keywords.find(arg_name) != NULL) {
							arg_name = arg_name.capitalize();
						}

						params_list += arg_type;
						params_list += " ";
						params_list += arg_name;
						params += arg_name;
						if (i < mi.arguments.size() -1) {
							params += ", ";
							params_list += ", ";
						}
					}
					method_info["params_list"] = params_list;
					method_info["params"] = params.empty() ? params : String(", ") + params;
				}
				// Virtual methods
				if (mi.flags & MethodFlags::METHOD_FLAG_VIRTUAL) {
					if (vmethod_return_exp.has(ret_type)) {
						method_info["return"] = vmethod_return_exp[ret_type];
					} else if (ret_type.ends_with("@")) {
						method_info["return"] = "return null;";
					} else {
						method_info["return"] = String("return ") + ret_type + "();";
					}
					methods += VMETHOD_TEMPLATE.format(method_info);
				} else {
					methods += ret_type.ends_with("@") ? METHOD_RO_TEMPLATE.format(method_info) : METHOD_TEMPLATE.format(method_info);
				}
			}
			class_info["methods"] = methods;
		}

		if (ClassDB::is_parent_class(*class_key, ReferenceName))
			classes += REFERENCE_TEMPLATE.format(class_info);
		else if (*class_key == ObjectName)
			classes += OBJECT_TEMPLATE.format(class_info).replace("bindings::instance_class(bindings::id_Object)", "bindings::Object();");
		else
			classes += OBJECT_TEMPLATE.format(class_info);
	}
	Dictionary file_info;
	file_info["classes"] = classes;
	script_bindings = FILE_TEMPLATE.format(file_info);
	return script_bindings;
}

#endif

}
