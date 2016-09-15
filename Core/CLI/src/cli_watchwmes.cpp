/////////////////////////////////////////////////////////////////
// watch-wmes command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "agent.h"
#include "decide.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include "print.h"
#include "slot.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "xml.h"

using namespace cli;
using namespace sml;

int RemoveWme(agent* thisAgent, wme* pWme)
{
    //    wme *w, *w2;
    //    Symbol *id;
    //    slot *s;

    //    w = (wme *) the_wme;

    Symbol* pId = pWme->id;

    // remove w from whatever list of wmes it's on
    wme* pWme2;
    for (pWme2 = pId->id->input_wmes; pWme2 != NIL; pWme2 = pWme2->next)
        if (pWme == pWme2)
        {
            break;
        }

    if (pWme2)
    {
        remove_from_dll(pId->id->input_wmes, pWme, next, prev);
    }

    for (pWme2 = pId->id->impasse_wmes; pWme2 != NIL; pWme2 = pWme2->next)
        if (pWme == pWme2)
        {
            break;
        }

    if (pWme2)
    {
        remove_from_dll(pId->id->impasse_wmes, pWme, next, prev);
    }

    slot* s;
    for (s = pId->id->slots; s != NIL; s = s->next)
    {

        for (pWme2 = s->wmes; pWme2 != NIL; pWme2 = pWme2->next)
            if (pWme == pWme2)
            {
                break;
            }

        if (pWme2)
        {
            remove_from_dll(s->wmes, pWme, next, prev);
        }

        for (pWme2 = s->acceptable_preference_wmes; pWme2 != NIL; pWme2 = pWme2->next)
            if (pWme == pWme2)
            {
                break;
            }

        if (pWme2)
        {
            remove_from_dll(s->acceptable_preference_wmes, pWme, next, prev);
        }
    }

#ifdef USE_CAPTURE_REPLAY
    // TODO: ommitted
#endif // USE_CAPTURE_REPLAY

    /* REW: begin 09.15.96 */
    if (pWme->gds)
    {
        if (pWme->gds->goal != NIL)
        {
            gds_invalid_so_remove_goal(thisAgent, pWme);
            /* NOTE: the call to remove_wme_from_wm will take care of checking if
            GDS should be removed */
        }
    }
    /* REW: end   09.15.96 */

    // now remove w from working memory
    remove_wme_from_wm(thisAgent, pWme);

    /* REW: begin 28.07.96 */
    /* See AddWme for description of what's going on here */

    if (thisAgent->current_phase != INPUT_PHASE)
    {
#ifndef NO_TIMING_STUFF
        thisAgent->timers_kernel.start();
#ifndef KERNEL_TIME_ONLY
        thisAgent->timers_phase.start();
#endif // KERNEL_TIME_ONLY
#endif // NO_TIMING_STUFF

        /* do_buffered_wm_and_ownership_changes(); */

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        thisAgent->timers_phase.stop();
        thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
#endif // KERNEL_TIME_ONLY
        thisAgent->timers_kernel.stop();
        thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
        thisAgent->timers_kernel.start();
#endif // NO_TIMING_STUFF
    }

/* This was previously using #ifndef NO_TOP_LEVEL_REFS, which is a macro constant that
 * no longer exists.  We now use DO_TOP_LEVEL_REF_CTS.  Top level refcounting is now
 * also disabled by default so changing it to #ifdef DO_TOP_LEVEL_REF_CTS would
 * change the current behavior.  Other uses of DO_TOP_LEVEL_REF_CTS seem to only be used
 * when adding refcounts to top-state wme's, so I'm not sure why the old macro prevented
 * this entire call.  So, I'm just going to comment it out for now and preserve existing
 * behavior. */
//#ifdef DO_TOP_LEVEL_REF_CTS
    do_buffered_wm_and_ownership_changes(thisAgent);
//#endif // DO_TOP_LEVEL_REF_CTS

    return 0;
}

bool read_wme_filter_component(agent* thisAgent, const char* s, Symbol** sym)
{
    soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, const_cast<char*>(s));
    if (lexeme.type == IDENTIFIER_LEXEME)
    {
        if ((*sym = thisAgent->symbolManager->find_identifier(lexeme.id_letter, lexeme.id_number)) == NIL)
        {
            return false;          /* Identifier does not exist */
        }
    }
    else
    {
        *sym = make_symbol_for_lexeme(thisAgent, &lexeme, false);
    }
    // Added by voigtjr because if this function can
    // legally return success with *sym == 0, my logic in AddWmeFilter will be broken.
    assert(*sym);
    return true;
}

int AddWMEFilter(agent* thisAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool adds, bool removes)
{
    Symbol* pId = 0;
    if (!read_wme_filter_component(thisAgent, pIdString, &pId))
    {
        return -1;
    }

    Symbol* pAttr = 0;
    if (!read_wme_filter_component(thisAgent, pAttrString, &pAttr))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        return -2;
    }

    Symbol* pValue = 0;
    if (!read_wme_filter_component(thisAgent, pValueString, &pValue))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        thisAgent->symbolManager->symbol_remove_ref(&pAttr);
        return -3;
    }

    /* check to see if such a filter has already been added: */
    cons* c;
    wme_filter* existing_wf;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {

        existing_wf = static_cast<wme_filter*>(c->first);

        // check for duplicate
        if ((existing_wf->adds == adds)
                && (existing_wf->removes == removes)
                && (existing_wf->id == pId)
                && (existing_wf->attr == pAttr)
                && (existing_wf->value == pValue))
        {
            thisAgent->symbolManager->symbol_remove_ref(&pId);
            thisAgent->symbolManager->symbol_remove_ref(&pAttr);
            thisAgent->symbolManager->symbol_remove_ref(&pValue);
            return -4; // Filter already exists
        }
    }

    wme_filter* wf = static_cast<wme_filter*>(thisAgent->memoryManager->allocate_memory(sizeof(wme_filter), MISCELLANEOUS_MEM_USAGE));
    wf->id = pId;
    wf->attr = pAttr;
    wf->value = pValue;
    wf->adds = adds;
    wf->removes = removes;

    /* Rather than add refs for the new filter symbols and then remove refs
    * for the identical symbols created from the string parameters, skip
    * the two nullifying steps altogether and just return immediately
    * after pushing the new filter:
    */
    push(thisAgent, wf, thisAgent->wme_filter_list);
    return 0;
}

int RemoveWMEFilter(agent* thisAgent, const char* pIdString, const char* pAttrString, const char* pValueString, bool adds, bool removes)
{
    Symbol* pId = 0;
    if (!read_wme_filter_component(thisAgent, pIdString, &pId))
    {
        return -1;
    }

    Symbol* pAttr = 0;
    if (!read_wme_filter_component(thisAgent, pAttrString, &pAttr))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        return -2;
    }

    Symbol* pValue = 0;
    if (!read_wme_filter_component(thisAgent, pValueString, &pValue))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pId);
        thisAgent->symbolManager->symbol_remove_ref(&pAttr);
        return -3;
    }

    cons* c;
    cons** prev_cons_rest = &thisAgent->wme_filter_list;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {
        wme_filter* wf = static_cast<wme_filter*>(c->first);

        // check for duplicate
        if ((wf->adds == adds)
                && (wf->removes == removes)
                && (wf->id == pId)
                && (wf->attr == pAttr)
                && (wf->value == pValue))
        {
            *prev_cons_rest = c->rest;
            thisAgent->symbolManager->symbol_remove_ref(&pId);
            thisAgent->symbolManager->symbol_remove_ref(&pAttr);
            thisAgent->symbolManager->symbol_remove_ref(&pValue);
            thisAgent->memoryManager->free_memory(wf, MISCELLANEOUS_MEM_USAGE);
            free_cons(thisAgent, c);
            return 0; /* assume that AddWMEFilter did not add duplicates */
        }
        prev_cons_rest = &(c->rest);
    }
    assert(!c);
    thisAgent->symbolManager->symbol_remove_ref(&pId);
    thisAgent->symbolManager->symbol_remove_ref(&pAttr);
    thisAgent->symbolManager->symbol_remove_ref(&pValue);
    return -4;
}

bool ResetWMEFilters(agent* thisAgent, bool adds, bool removes)
{
    cons* c;
    bool didRemoveSome = false;
    cons** prev_cons_rest = &thisAgent->wme_filter_list;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {

        wme_filter* wf = static_cast<wme_filter*>(c->first);
        if ((adds && wf->adds) || (removes && wf->removes))
        {
            *prev_cons_rest = c->rest;
            thisAgent->outputManager->printa_sf(thisAgent, "Removed: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
            thisAgent->outputManager->printa_sf(thisAgent,  "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
            thisAgent->symbolManager->symbol_remove_ref(&wf->id);
            thisAgent->symbolManager->symbol_remove_ref(&wf->attr);
            thisAgent->symbolManager->symbol_remove_ref(&wf->value);
            thisAgent->memoryManager->free_memory(wf, MISCELLANEOUS_MEM_USAGE);
            free_cons(thisAgent, c);
            didRemoveSome = true;
        }
        prev_cons_rest = &(c->rest);
    }
    return didRemoveSome;
}

void ListWMEFilters(agent* thisAgent, bool adds, bool removes)
{
    cons* c;
    for (c = thisAgent->wme_filter_list; c != NIL; c = c->rest)
    {
        wme_filter* wf = static_cast<wme_filter*>(c->first);

        if ((adds && wf->adds) || (removes && wf->removes))
        {
            thisAgent->outputManager->printa_sf(thisAgent, "wme filter: (%y ^%y %y) ", wf->id, wf->attr, wf->value);
            thisAgent->outputManager->printa_sf(thisAgent,  "%s %s\n", (wf->adds ? "adds" : ""), (wf->removes ? "removes" : ""));
        }
    }
}

bool CommandLineInterface::DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString)
{
    int ret = 0;
    bool retb = false;
    switch (mode)
    {
        case WATCH_WMES_ADD:
            if (!pIdString || !pAttributeString || !pValueString)
            {
                return SetError("ID/Attribute/Value filter expected, one or more missing.");
            }
            ret = AddWMEFilter(m_pAgentSML->GetSoarAgent(), pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
            if (ret == -1)
            {
                return SetError("Invalid id, got: " + *pIdString);
            }
            if (ret == -2)
            {
                return SetError("Invalid attribute, got: " + *pAttributeString);
            }
            if (ret == -3)
            {
                return SetError("Invalid value, got: " + *pValueString);
            }
            if (ret == -4)
            {
                return SetError("That WME filter already exists.");
            }
            break;

        case WATCH_WMES_REMOVE:
            if (!pIdString || !pAttributeString || !pValueString)
            {
                return SetError("ID/Attribute/Value filter expected, one or more missing.");
            }
            ret = RemoveWMEFilter(m_pAgentSML->GetSoarAgent(), pIdString->c_str(), pAttributeString->c_str(), pValueString->c_str(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
            if (ret == -1)
            {
                return SetError("Invalid id, got: " + *pIdString);
            }
            if (ret == -2)
            {
                return SetError("Invalid attribute, got: " + *pAttributeString);
            }
            if (ret == -3)
            {
                return SetError("Invalid value, got: " + *pValueString);
            }
            if (ret == -4)
            {
                return SetError("The specified WME filter was not found.");
            }
            break;

        case WATCH_WMES_LIST:
            if (type.none())
            {
                type.flip();
            }

            ListWMEFilters(m_pAgentSML->GetSoarAgent(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));
            break;

        case WATCH_WMES_RESET:
            if (type.none())
            {
                type.flip();
            }

            retb = ResetWMEFilters(m_pAgentSML->GetSoarAgent(), type.test(WATCH_WMES_TYPE_ADDS), type.test(WATCH_WMES_TYPE_REMOVES));

            if (!retb)
            {
                return SetError("The specified WME filter was not found.");
            }
            break;

        default:
            return SetError("Invalid mode.");
    }

    return true;
}
