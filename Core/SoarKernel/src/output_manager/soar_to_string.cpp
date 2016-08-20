/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
             soar_to_string.cpp
------------------------------------------------------------------ */

#include "output_manager.h"

#include "ebc.h"
#include "rhs.h"
#include "print.h"
#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "lexer.h"
#include "rete.h"
#include "production_reorder.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "preference.h"
#include "working_memory.h"
#include "soar_instance.h"
#include "test.h"

bool Output_Manager::wme_to_string(agent* thisAgent, wme* w, std::string &destString)
{
    assert(thisAgent && w);

    sprinta_sf(thisAgent, destString, "(t%u(r%u): %y(l%d) ^%y %y(l%d)%s",
        w->timetag, w->reference_count, w->id, w->id->id->level, w->attr, w->value, w->value->is_identifier() ? w->value->id->level : 0,
        (w->acceptable ? " +)" : ")"));

    /* This is a bool, b/c sometimes we limit printing of WM to certain wme's.
     * Return value used to determine whether to print newline*/
    return true;
}

void Output_Manager::WM_to_string(agent* thisAgent, std::string &destString)
{
    assert(thisAgent);
    destString += "--------------------------- WMEs --------------------------\n";
    for (wme* w = m_defaultAgent->all_wmes_in_rete; w != NIL; w = w->rete_next)
    {
//        if (m_pre_string) destString += m_pre_string;
        if (wme_to_string(thisAgent, w, destString))
        {
            destString += '\n';
        }
    }
    return;
}

const char* Output_Manager::test_type_to_string_brief(byte test_type)
{
    switch (test_type)
    {
        case NOT_EQUAL_TEST:
            return "<> ";
            break;
        case LESS_TEST:
            return "< ";
            break;
        case GREATER_TEST:
            return "> ";
            break;
        case LESS_OR_EQUAL_TEST:
            return "<= ";
            break;
        case GREATER_OR_EQUAL_TEST:
            return ">= ";
            break;
        case SAME_TYPE_TEST:
            return "<=> ";
            break;
        case CONJUNCTIVE_TEST:
            return "{ }";
            break;
        case GOAL_ID_TEST:
            return "IS_G_ID ";
            break;
        case IMPASSE_ID_TEST:
            return "IS_IMPASSE ";
            break;
        case EQUALITY_TEST:
            return "= ";
            break;
    }
    return "UNDEFINED TEST TYPE";
}

void Output_Manager::test_to_string(test t, std::string &destString, bool show_equality)
{
    cons* c;
    if (!t)
    {
        destString += "[BLANK TEST]";

        return;
    }

    if (t->type == EQUALITY_TEST)
    {
        if (show_equality)
        {
            destString += test_type_to_string_brief(t->type);
        }
        destString += t->data.referent->to_string(false);
    }
    else if (t->type == CONJUNCTIVE_TEST)
    {
        destString += "{ ";
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {

            this->test_to_string(static_cast<test>(c->first), destString, show_equality);
            destString += ' ';
        }
        destString += '}';
    }
    else if (test_has_referent(t))
    {
        destString += test_type_to_string_brief(t->type);
        destString += t->data.referent->to_string(false);
    }
    else if (t->type == DISJUNCTION_TEST)
    {
        destString += "<< ";
        for (c = t->data.disjunction_list; c != NIL; c = c->rest)
        {
            destString += static_cast<symbol_struct*>(c->first)->to_string(false);
            destString += ' ';
        }
        destString += ">>";
    }
    else if (t->type == GOAL_ID_TEST)
    {
        destString += "state";
    }
    else if (t->type == IMPASSE_ID_TEST)
    {
        destString += "impasse";
    } else {
        destString += "[INVALID TEST!!!]";
    }
    return;
}


void Output_Manager::condition_cons_to_string(agent* thisAgent, cons* c, std::string &destString)
{
    while (c)
    {
        sprinta_sf(thisAgent, destString, "%s: %l\n", m_pre_string, static_cast<condition_struct*>(c->first));
        c = c->rest;
    }
    return;
}

void Output_Manager::condition_to_string(agent* thisAgent, condition* cond, std::string &destString)
{
    if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        if (m_print_actual_effective)
        {
            sprinta_sf(thisAgent, destString, "(%t %s^%t %t)",
            cond->data.tests.id_test,
                (cond->type == NEGATIVE_CONDITION) ? "- ": NULL,
            cond->data.tests.attr_test, cond->data.tests.value_test);
        }
        if (m_print_identity_effective) {
            sprinta_sf(thisAgent, destString, "%s(%g %s^%g %g)",
                m_print_actual_effective ? ", " : NULL,
                cond->data.tests.id_test,
                (cond->type == NEGATIVE_CONDITION) ? "- ": NULL,
                cond->data.tests.attr_test, cond->data.tests.value_test);
        }
    }
    else
    {
        sprinta_sf(thisAgent, destString, "-{\n%1}", cond->data.ncc.top);
    }
    return;
}

void Output_Manager::condition_list_to_string(agent* thisAgent, condition* top_cond, std::string &destString)
{

    condition* cond;
    int64_t count = 0;

    for (cond = top_cond; cond != NIL; cond = cond->next)
    {
        assert(cond != cond->next);
        sprinta_sf(thisAgent, destString, "%s%i: %l\n", m_pre_string, ++count, cond);
    }
    return;
}

void Output_Manager::rhs_value_to_string(agent* thisAgent, rhs_value rv, std::string &destString, struct token_struct* tok, wme* w, bool pEmptyStringForNullIdentity)
{
    rhs_symbol rsym = NIL;
    Symbol* sym = NIL;
    cons* c;
    list* fl;
    rhs_function* rf;

    if (!rv)
    {
        destString += '#';
    }
    else if (rhs_value_is_unboundvar(rv))
    {
        /* -- unbound variable -- */
        destString += "[STI_UV]";

    }
    else if (rhs_value_is_symbol(rv))
    {
        /* -- rhs symbol -- */
        rsym = rhs_value_to_rhs_symbol(rv);
        if (this->m_print_actual_effective || (!pEmptyStringForNullIdentity && (!rsym->o_id)))
        {
            if (rsym->referent)
            {
                destString += rsym->referent->to_string(false);
            } else {
                destString += '#';
            }
        }
        if (m_print_identity_effective && rsym->o_id) {
            sprinta_sf(thisAgent, destString, " (%u)", rsym->o_id);
        }
    }
    else if (rhs_value_is_reteloc(rv))
    {
        /* -- rete location (cannot get symbol without token information) -- */
        if (tok && w)
        {
            sym = get_symbol_from_rete_loc(
                rhs_value_to_reteloc_levels_up(rv),
                rhs_value_to_reteloc_field_num(rv), tok, w);
            if (sym)
            {
                destString += sym->to_string(false);
            } else {
                destString += "[STI-RETE]";
            }
        }
        else
        {
            destString += "[STI]";
        }
    }
    else
    {
        assert(rhs_value_is_funcall(rv));
        /* -- function call -- */
        fl = rhs_value_to_funcall_list(rv);
        rf = static_cast<rhs_function_struct*>(fl->first);

        destString += '(';
        if (rf->name)
        {
            destString += rf->name->to_string(false);
        } else {
            destString += '#';
        }
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            destString += ' ';
            rhs_value_to_string(thisAgent, static_cast<char*>(c->first), destString, tok, w, false);
        }
        destString += ')';
    }
}

void Output_Manager::action_to_string(agent* thisAgent, action* a, std::string &destString)
{
    assert(thisAgent && a);
    if (a->type == FUNCALL_ACTION)
    {
        if (m_pre_string) destString += m_pre_string;
        destString += "(rhs_function ";
        rhs_value_to_string(thisAgent, a->value, destString);
        destString += ')';
    } else {
        if (m_pre_string) destString += m_pre_string;
        destString += '(';
        rhs_value_to_string(thisAgent, a->id, destString);
        destString += " ^";
        rhs_value_to_string(thisAgent, a->attr, destString);
        destString += ' ';
        rhs_value_to_string(thisAgent, a->value, destString);
        destString += " ";
        destString += preference_to_char(a->preference_type);
        if (a->referent)
        {
            destString += " ";
            rhs_value_to_string(thisAgent, a->referent, destString);
        }
        destString += ')';
    }
}

void Output_Manager::action_list_to_string(agent* thisAgent, action* action_list, std::string &destString)
{
    assert(thisAgent && action_list);
    action* a = NIL;

    for (a = action_list; a != NIL; a = a->next)
    {
        action_to_string(thisAgent, a, destString);
        destString += '\n';
    }
}

void Output_Manager::pref_to_string(agent* thisAgent, preference* pref, std::string &destString)
{
    assert(thisAgent && pref);
    if (m_print_actual_effective)
    {
        sprinta_sf(thisAgent, destString, "(%y ^%y %y) %c", pref->id, pref->attr, pref->value, preference_to_char(pref->type));
        if (preference_is_binary(pref->type))
        {
            sprinta_sf(thisAgent, destString, " %y%s", pref->referent);
        }
    }
    if (m_print_identity_effective)
    {
        std::string lID, lAttr, lValue;
        if (pref->o_ids.id) {
            lID = "<" + std::to_string(pref->o_ids.id) + ">";
        } else {
            lID = pref->id->to_string(false);
        }
        if (pref->o_ids.attr) {
            lAttr = "<" + std::to_string(pref->o_ids.attr) + ">";
        } else {
            lAttr = pref->attr->to_string(false);
        }
        if (pref->o_ids.value) {
            lValue = "<" + std::to_string(pref->o_ids.value) + ">";
        } else {
            lValue = pref->value->to_string(false);
        }
        sprinta_sf(thisAgent, destString, "%s(%s ^%s %s) %c", (m_print_actual_effective) ? ", " : "",
            lID.c_str(), lAttr.c_str(), lValue.c_str(), preference_to_char(pref->type));

        if (preference_is_binary(pref->type))
        {
            sprinta_sf(thisAgent, destString, " %y", pref->referent);
        }
        if (pref->o_supported)
        {
            sprinta_sf(thisAgent, destString, " (o-support)");
        } else {
            sprinta_sf(thisAgent, destString, " (i-support)");
        }
    }
}

void Output_Manager::preflist_inst_to_string(agent* thisAgent, preference* top_pref, std::string &destString)
{
    for (preference* pref = top_pref; pref != NIL;)
    {
        sprinta_sf(thisAgent, destString, "%s%p\n", m_pre_string, pref);
        pref = pref->inst_next;
    }
}

void Output_Manager::preflist_result_to_string(agent* thisAgent, preference* top_pref, std::string &destString)
{
    for (preference* pref = top_pref; pref != NIL;)
    {
        sprinta_sf(thisAgent, destString, "%s%p\n", m_pre_string, pref);
        pref = pref->next_result;
    }
}

void Output_Manager::debug_print_production(TraceMode mode, production* prod)
{
    if (!is_debug_mode_enabled(mode)) return;
    if (!m_defaultAgent) return;

    if (prod)
    {
        print_production(m_defaultAgent, prod, true);
    }
}

void Output_Manager::cond_prefs_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, std::string &destString)
{
    if (m_print_actual)
    {
        if (m_print_identity)
        {
            destString += "--------------------------- Match --------------------------\n";
        }
        set_print_test_format(true, false);
        condition_list_to_string(thisAgent, top_cond, destString);
        if (m_pre_string) destString += m_pre_string;
        destString += "-->\n";
        preflist_inst_to_string(thisAgent, top_pref, destString);
        clear_print_test_format();
    }
    if (m_print_identity)
    {
        if (m_print_actual)
        {
            destString += "------------------------- Identity -------------------------\n";
        }
        set_print_test_format(false, true);
        condition_list_to_string(thisAgent, top_cond, destString);
        if (m_pre_string) destString += m_pre_string;
        destString += "-->\n";
        preflist_inst_to_string(thisAgent, top_pref, destString);
        clear_print_test_format();
    }
}

void Output_Manager::cond_results_to_string(agent* thisAgent, condition* top_cond, preference* top_pref, std::string &destString)
{
    if (m_print_actual)
    {
        if (m_print_identity)
        {
            destString += "--------------------------- Match --------------------------\n";
        }
        set_print_test_format(true, false);
        condition_list_to_string(thisAgent, top_cond, destString);
        if (m_pre_string) destString += m_pre_string;
        destString += "-->\n";
        preflist_result_to_string(thisAgent, top_pref, destString);
        clear_print_test_format();
    }
    if (m_print_identity)
    {
        if (m_print_actual)
        {
            destString += "------------------------- Identity -------------------------\n";
        }
        set_print_test_format(false, true);
        condition_list_to_string(thisAgent, top_cond, destString);
        if (m_pre_string) destString += m_pre_string;
        destString += "-->\n";
        preflist_result_to_string(thisAgent, top_pref, destString);
        clear_print_test_format();
    }
}

void Output_Manager::cond_actions_to_string(agent* thisAgent, condition* top_cond, action* top_action, std::string &destString)
{
    if (m_print_actual)
    {
        if (m_print_identity)
        {
            sprinta_sf(thisAgent, destString, "--------------------------- Match --------------------------\n");
        }
        set_print_test_format(true, false);
        condition_list_to_string(thisAgent, top_cond, destString);
        sprinta_sf(thisAgent, destString, "%s-->\n", m_pre_string);
        action_list_to_string(thisAgent, top_action, destString);
        clear_print_test_format();
    }
    if (m_print_identity)
    {
        if (m_print_actual)
        {
            sprinta_sf(thisAgent, destString, "------------------------- Identity -------------------------\n");
            set_print_test_format(false, true);
        }
        condition_list_to_string(thisAgent, top_cond, destString);
        sprinta_sf(thisAgent, destString, "%s-->\n", m_pre_string);
        action_list_to_string(thisAgent, top_action, destString);
        clear_print_test_format();
    }
}

void Output_Manager::instantiation_to_string(agent* thisAgent, instantiation* inst, std::string &destString)
{
    sprinta_sf(thisAgent, destString, "%sInstantiation (i%u) matched %y in state %y (level %d)\n", 
        m_pre_string, inst->i_id, inst->prod_name, inst->match_goal, inst->match_goal_level);
    cond_prefs_to_string(thisAgent, inst->top_of_instantiated_conditions, inst->preferences_generated, destString);
}

void add_inst_of_type(agent* thisAgent, unsigned int productionType, std::vector<instantiation*>& instantiation_list)
{
    for (production* prod = thisAgent->all_productions_of_type[productionType]; prod != NIL; prod = prod->next)
    {
        for (instantiation* inst = prod->instantiations; inst != NIL; inst = inst->next)
        {
            instantiation_list.push_back(inst);
        }
    }
}

void Output_Manager::print_all_inst(TraceMode mode)
{
    if (!is_debug_mode_enabled(mode)) return;
    if (!m_defaultAgent) return;

    print( "--- Instantiations: ---\n");

    std::vector<instantiation*> instantiation_list;
    add_inst_of_type(m_defaultAgent, CHUNK_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, DEFAULT_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, JUSTIFICATION_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, USER_PRODUCTION_TYPE, instantiation_list);
    add_inst_of_type(m_defaultAgent, TEMPLATE_PRODUCTION_TYPE, instantiation_list);

    for (int y = 0; y < instantiation_list.size(); y++)
    {
        print_sf("- Instantiation %d:\n", y);
        print_sf("%7", instantiation_list[y]);
    }
}

void Output_Manager::print_saved_test(TraceMode mode, saved_test* st)
{
    if (!is_debug_mode_enabled(mode)) return;

    print_sf("  Index: %y  Test: %t\n", st->var, st->the_test);
}

void Output_Manager::print_saved_test_list(TraceMode mode, saved_test* st)
{
    if (!is_debug_mode_enabled(mode)) return;

    while (st)
    {
        print_saved_test(mode, st);
        st = st->next;
    }
}

void Output_Manager::print_varnames(TraceMode mode, varnames* var_names)
{
    cons* c;;

    if (!is_debug_mode_enabled(mode)) return;

    if (!var_names)
    {
        print("None.");;
    }
    else if (varnames_is_one_var(var_names))
    {
        print_sf("%y ", varnames_to_one_var(var_names));;
    }
    else
    {
        c = varnames_to_var_list(var_names);
        while (c)
        {
            print_sf("%y ", static_cast<Symbol*>(c->first));;
            c = c->rest;
        }
    }
}
void Output_Manager::print_varnames_node(TraceMode mode, node_varnames* var_names_node)
{

    if (!is_debug_mode_enabled(mode)) return;

    if (!var_names_node)
    {
        print("varnames node empty.\n");
    }
    else
    {
        print("varnames for node = ID: ");

        print_varnames(mode, var_names_node->data.fields.id_varnames);
        print(" | Attr: ");
        print_varnames(mode, var_names_node->data.fields.attr_varnames);
        print(" | Value: ");
        print_varnames(mode, var_names_node->data.fields.value_varnames);
        print("\n");
    }
}

void Output_Manager::debug_find_and_print_sym(char* find_string)
{
    Symbol* newSym = NULL;
    if (find_string)
    {
        bool found = false;
        bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
        bool rereadable;
        std::string convertStr(find_string);
        std::stringstream convert(convertStr);
        int newInt;
        double newFloat;

        if (!m_defaultAgent)
        {
            return;
        }

        soar::Lexer::determine_possible_symbol_types_for_string(find_string,
                static_cast<size_t>(strlen(find_string)),
                &possible_id,
                &possible_var,
                &possible_sc,
                &possible_ic,
                &possible_fc,
                &rereadable);

        if (possible_id)
        {
            newSym = find_identifier(m_defaultAgent, toupper(find_string[0]), strtol(&find_string[1], NULL, 10));
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_var)
        {
            newSym = find_variable(m_defaultAgent, find_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_sc)
        {
            newSym = find_str_constant(m_defaultAgent, find_string);
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_ic)
        {
            if (convert >> newInt)
            {
                newSym = find_int_constant(m_defaultAgent, newInt);
            }
            if (newSym)
            {
                found = true;
            }
        }
        if (!found && possible_fc)
        {
            if (convert >> newFloat)
            {
                newSym = find_float_constant(m_defaultAgent, newFloat);
            }
            if (newSym)
            {
                found = true;
            }
        }
    }
    if (newSym)
    {
        debug_print_sf(DT_DEBUG,
               "%y:\n"
               "  type     = %d\n"
               "  refcount = %d\n"
               "  tc_num   = %d\n",
               newSym,
               newSym->symbol_type,
               newSym->reference_count,
               newSym->tc_num);
    }
    else
    {
        debug_print_sf(DT_DEBUG, "No symbol %s found.\n", find_string);
    }
}

bool om_print_sym(agent* thisAgent, void* item, void* vMode)
{
    TraceMode mode = * static_cast < TraceMode* >(vMode);

    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return false;

    Output_Manager::Get_OM().printa_sf(thisAgent, "%y (%i)\n", static_cast<symbol_struct*>(item), static_cast<symbol_struct*>(item)->reference_count);
    return false;
}

void Output_Manager::print_identifiers(TraceMode mode)
{
    if (!is_debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    print("--- Identifiers: ---\n");
    do_for_all_items_in_hash_table(m_defaultAgent, m_defaultAgent->identifier_hash_table, om_print_sym, &mode);
}

void Output_Manager::print_variables(TraceMode mode)
{
    if (!is_debug_mode_enabled(mode)) return;

    if (!m_defaultAgent) return;

    print("--- Variables: ---\n");
    do_for_all_items_in_hash_table(m_defaultAgent, m_defaultAgent->variable_hash_table, om_print_sym, &mode);
}

void debug_print_db_err(TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
    if (!thisAgent) return;

    print_sysparam_trace(thisAgent, 0, "Debug| Printing database status/errors...\n");
//  if (thisAgent->debug_params->epmem_commands->get_value() == on)
//  {
//    if (!db_err_epmem_db)
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access epmem database because wmg not yet initialized.\n");
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| EpMem DB: %d - %s\n", sqlite3_errcode( db_err_epmem_db->get_db() ),
//          sqlite3_errmsg( db_err_epmem_db->get_db() ));
//    }
//  }
//  if (thisAgent->debug_params->smem_commands->get_value() == on)
//  {
//    if (!db_err_smem_db)
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access smem database because wmg not yet initialized.\n");
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| SMem DB: %d - %s\n", sqlite3_errcode( db_err_smem_db->get_db() ),
//          sqlite3_errmsg( db_err_smem_db->get_db() ));
//    }
//  }
}

void debug_print_epmem_table(const char* table_name, TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    //agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
//    if (!thisAgent) return;

//  if (!db_err_epmem_db)
//  {
//    if ((thisAgent->epmem_db) && ( thisAgent->epmem_db->get_status() == soar_module::connected ))
//    {
//      db_err_epmem_db = m_defaultAgent->epmem_db;
//      thisAgent->debug_params->epmem_commands->set_value(on);
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access epmem database because database not yet initialized.\n");
//      return;
//    }
//  }
//
//  db_err_epmem_db->print_table(table_name);
}

void debug_print_smem_table(const char* table_name, TraceMode mode)
{
    if (!Output_Manager::Get_OM().is_debug_mode_enabled(mode)) return;
    //agent* thisAgent = Output_Manager::Get_OM().get_default_agent();
//    if (!thisAgent) return;

//  if (!db_err_smem_db)
//  {
//    if (thisAgent->smem_db && ( thisAgent->smem_db->get_status() == soar_module::connected ))
//    {
//      db_err_smem_db = m_defaultAgent->smem_db;
//      thisAgent->debug_params->smem_commands->set_value(on);
//    }
//    else
//    {
//      print_trace (thisAgent,0, "Debug| Cannot access smem database because database not yet initialized.\n");
//      return;
//    }
//  }
//  db_err_smem_db->print_table(table_name);
}

void Output_Manager::print_current_lexeme(TraceMode mode, soar::Lexer* lexer)
{
    std::string lex_type_string;

    if (!is_debug_mode_enabled(mode)) return;

    switch (lexer->current_lexeme.type)
    {
        case EOF_LEXEME:
            lex_type_string = "EOF_LEXEME";
            break;
        case IDENTIFIER_LEXEME:
            lex_type_string = "IDENTIFIER_LEXEME";
            break;
        case VARIABLE_LEXEME:
            lex_type_string = "VARIABLE_LEXEME";
            break;
        case STR_CONSTANT_LEXEME:
            lex_type_string = "STR_CONSTANT_LEXEME";
            break;
        case INT_CONSTANT_LEXEME:
            lex_type_string = "INT_CONSTANT_LEXEME";
            break;
        case FLOAT_CONSTANT_LEXEME:
            lex_type_string = "FLOAT_CONSTANT_LEXEME";
            break;
        case L_PAREN_LEXEME:
            lex_type_string = "L_PAREN_LEXEME";
            break;
        case R_PAREN_LEXEME:
            lex_type_string = "R_PAREN_LEXEME";
            break;
        case L_BRACE_LEXEME:
            lex_type_string = "L_BRACE_LEXEME";
            break;
        case R_BRACE_LEXEME:
            lex_type_string = "R_BRACE_LEXEME";
            break;
        case PLUS_LEXEME:
            lex_type_string = "PLUS_LEXEME";
            break;
        case MINUS_LEXEME:
            lex_type_string = "MINUS_LEXEME";
            break;
        case RIGHT_ARROW_LEXEME:
            lex_type_string = "RIGHT_ARROW_LEXEME";
            break;
        case GREATER_LEXEME:
            lex_type_string = "GREATER_LEXEME";
            break;
        case LESS_LEXEME:
            lex_type_string = "LESS_LEXEME";
            break;
        case EQUAL_LEXEME:
            lex_type_string = "EQUAL_LEXEME";
            break;
        case LESS_EQUAL_LEXEME:
            lex_type_string = "LESS_EQUAL_LEXEME";
            break;
        case GREATER_EQUAL_LEXEME:
            lex_type_string = "GREATER_EQUAL_LEXEME";
            break;
        case NOT_EQUAL_LEXEME:
            lex_type_string = "NOT_EQUAL_LEXEME";
            break;
        case LESS_EQUAL_GREATER_LEXEME:
            lex_type_string = "LESS_EQUAL_GREATER_LEXEME";
            break;
        case LESS_LESS_LEXEME:
            lex_type_string = "LESS_LESS_LEXEME";
            break;
        case GREATER_GREATER_LEXEME:
            lex_type_string = "GREATER_GREATER_LEXEME";
            break;
        case AMPERSAND_LEXEME:
            lex_type_string = "AMPERSAND_LEXEME";
            break;
        case AT_LEXEME:
            lex_type_string = "AT_LEXEME";
            break;
        case TILDE_LEXEME:
            lex_type_string = "TILDE_LEXEME";
            break;
        case UP_ARROW_LEXEME:
            lex_type_string = "UP_ARROW_LEXEME";
            break;
        case EXCLAMATION_POINT_LEXEME:
            lex_type_string = "EXCLAMATION_POINT_LEXEME";
            break;
        case COMMA_LEXEME:
            lex_type_string = "COMMA_LEXEME";
            break;
        case PERIOD_LEXEME:
            lex_type_string = "PERIOD_LEXEME";
            break;
        case QUOTED_STRING_LEXEME:
            lex_type_string = "QUOTED_STRING_LEXEME";
            break;
        case DOLLAR_STRING_LEXEME:
            lex_type_string = "DOLLAR_STRING_LEXEME";
            break;
        case NULL_LEXEME:
            lex_type_string = "NULL_LEXEME";
            break;
        default:
            break;
    }
    print_sf( "%s: \"%s\"\n", lex_type_string.c_str(), lexer->current_lexeme.string());

}
