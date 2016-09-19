/////////////////////////////////////////////////////////////////
// explain command file.
//
// Author: Mazin Assanie
// Date  : 2015
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"

#include "agent.h"
#include "condition.h"
#include "explanation_memory.h"
#include "misc.h"
#include "output_manager.h"

#include <string>

using namespace cli;
using namespace sml;

bool is_explain_id(const std::string* pStringParameter)
{
    return false;
}

bool CommandLineInterface::DoExplain(ExplainBitset options, const std::string* pStringParameter, const std::string* pStringParameter2)
{

    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    bool lReturn_Value = false;

    /* Handle options that enable/disable recording of chunk formation */
    if (options.test(EXPLAIN_ALL))
    {
        thisAgent->explanationMemory->set_enabled(true);
        thisAgent->outputManager->printa_sf(thisAgent, "Will monitor all chunks created.\n");
        return true;
    }
    if (options.test(EXPLAIN_ONLY_SPECIFIC))
    {
        thisAgent->explanationMemory->set_enabled(false);
        thisAgent->outputManager->printa_sf(thisAgent, "Will only monitor specific chunks.\n");
        return true;
    }
    if (options.test(EXPLAIN_JUSTIFICATIONS))
    {
        if (pStringParameter->empty())
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Soar is%scurrently recording justifications.\n", thisAgent->explanationMemory->isRecordingJustifications() ? " " : " not ");
        } else {
            if (pStringParameter->at(0) == 'y')
            {
                thisAgent->explanationMemory->set_justifications_enabled(true);
            } else if (pStringParameter->at(0) == 'n')
            {
                thisAgent->explanationMemory->set_justifications_enabled(true);
            } else {
                thisAgent->outputManager->printa_sf(thisAgent, "Invalid argument '%s' to explain --justifications.\n", pStringParameter->c_str());
            }
        }
        return true;
    }
    /* Handle options that required a currently discussed chunk/justification */
    if (!thisAgent->explanationMemory->current_discussed_chunk_exists() && (options.test(EXPLAIN_FORMATION) || options.test(EXPLAIN_CONSTRAINTS) ||
        options.test(EXPLAIN_IDENTITY_SETS) || options.test(EXPLAIN_STATS) || options.test(EXPLAIN_EXPLANATION_TRACE) || options.test(EXPLAIN_WME_TRACE)))
    {
        SetError("Please first specify the chunk you want to discuss with the command 'explain [chunk-name]' or 'explain chunk [chunk ID]'.");
        return false;
    }
    else
    {
            if (options.test(EXPLAIN_FORMATION))
        {
            thisAgent->explanationMemory->print_formation_explanation();
        }
        if (options.test(EXPLAIN_CONSTRAINTS))
        {
            thisAgent->explanationMemory->print_constraints_enforced();
        }
        if (options.test(EXPLAIN_IDENTITY_SETS))
        {
            thisAgent->explanationMemory->print_identity_set_explanation();
        }
        if (options.test(EXPLAIN_STATS))
        {
            thisAgent->explanationMemory->print_chunk_stats();
        }
        if (options.test(EXPLAIN_EXPLANATION_TRACE))
        {
            thisAgent->explanationMemory->switch_to_explanation_trace(true);
        }
        if (options.test(EXPLAIN_WME_TRACE))
        {
            thisAgent->explanationMemory->switch_to_explanation_trace(false);
        }
    }

    /* Handle global stats command*/
    if (options.test(EXPLAIN_GLOBAL_STATS))
    {
            thisAgent->explanationMemory->print_explainer_stats();
            return true;
    }

    /* Handle global stats command*/
    if (options.test(EXPLAIN_LIST_ALL))
    {
            thisAgent->explanationMemory->print_all_chunks();
            return true;
    }

    /* Handle record command*/
    if (options.test(EXPLAIN_RECORD))
    {
        if (pStringParameter->empty())
        {
            thisAgent->explanationMemory->print_all_watched_rules();
        } else {
            return thisAgent->explanationMemory->watch_rule(pStringParameter);
        }
    }

    /* Handle non-option explain commands for rules and Soar data structures */
    if (!options.any())
    {
        if (pStringParameter->empty())
        {
                    thisAgent->explanationMemory->print_explain_summary();
                    return true;
        } else if (pStringParameter2->empty()) {
            return thisAgent->explanationMemory->explain_chunk(pStringParameter);
        } else {
            return thisAgent->explanationMemory->explain_item(pStringParameter, pStringParameter2);
        }
    } else {
        if (!pStringParameter->empty())
        {
            SetError("Those options cannot take additional arguments.  Ignoring.\n");
            return false;
        }
    }
    return false;
}

