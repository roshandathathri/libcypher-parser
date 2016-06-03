/* vi:set ts=4 sw=4 expandtab:
 *
 * Copyright 2016, Chris Leishman (http://github.com/cleishm)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../../config.h"
#include "result.h"
#include "ast.h"
#include "util.h"
#include <assert.h>


cypher_parse_result_t *cypher_parse_result(
        cypher_astnode_t * const *elements, unsigned int nelements,
        const cypher_parse_segment_t *segments, unsigned int nsegments,
        cypher_astnode_t * const *directives, unsigned int ndirectives,
        const cypher_parse_error_t *errors, unsigned int nerrors,
        bool eof, unsigned int initial_ordinal)
{
    cypher_parse_result_t *result = calloc(1, sizeof(cypher_parse_result_t));
    if (result == NULL)
    {
        return NULL;
    }

    result->elements = mdup(elements, nelements * sizeof(cypher_astnode_t *));
    if (nelements > 0 && result->elements == NULL)
    {
        goto failure;
    }
    result->nelements = nelements;

    result->segments = mdup(segments,
            nsegments * sizeof(cypher_parse_segment_t));
    if (nsegments > 0 && result->segments == NULL)
    {
        goto failure;
    }
    result->nsegments = nsegments;

    result->directives = mdup(directives,
            ndirectives * sizeof(cypher_astnode_t *));
    if (ndirectives > 0 && result->directives == NULL)
    {
        goto failure;
    }
    result->ndirectives = ndirectives;

    result->errors = mdup(errors, nerrors * sizeof(cypher_parse_error_t));
    if (nerrors > 0 && result->errors == NULL)
    {
        goto failure;
    }
    result->nerrors = nerrors;

    result->node_count = initial_ordinal;
    for (unsigned int i = 0; i < result->nelements; ++i)
    {
        result->node_count = cypher_ast_set_ordinals(
                result->elements[i], result->node_count);
    }

    result->eof = eof;

    return result;

    int errsv;
failure:
    errsv = errno;
    free(result->errors);
    free(result->directives);
    free(result->elements);
    free(result->segments);
    free(result);
    errno = errsv;
    return NULL;
}



unsigned int cypher_parse_result_nsegments(const cypher_parse_result_t *result)
{
    return result->nsegments;
}


const cypher_parse_segment_t *cypher_parse_result_segment(
        const cypher_parse_result_t *result, unsigned int index)
{
    if (index >= result->nsegments)
    {
        return NULL;
    }
    return &(result->segments[index]);
}


struct cypher_input_range cypher_parse_segment_range(
        const cypher_parse_segment_t *segment)
{
    return segment->range;
}


const cypher_astnode_t *cypher_parse_segment_directive(
        const cypher_parse_segment_t *segment)
{
    return segment->directive;
}


unsigned int cypher_parse_result_nelements(const cypher_parse_result_t *result)
{
    return result->nelements;
}


const cypher_astnode_t *cypher_parse_result_element(
        const cypher_parse_result_t *result, unsigned int index)
{
    if (index >= result->nelements)
    {
        return NULL;
    }
    return result->elements[index];
}


unsigned int cypher_parse_result_node_count(const cypher_parse_result_t *result)
{
    return result->node_count;
}


unsigned int cypher_parse_result_ndirectives(
        const cypher_parse_result_t *result)
{
    return result->ndirectives;
}


const cypher_astnode_t *cypher_parse_result_directive(
                const cypher_parse_result_t *result, unsigned int index)
{
    if (index >= result->ndirectives)
    {
        return NULL;
    }
    return result->directives[index];
}


unsigned int cypher_parse_result_nerrors(const cypher_parse_result_t *result)
{
    return result->nerrors;
}


const cypher_parse_error_t *cypher_parse_result_error(
        const cypher_parse_result_t *result, unsigned int index)
{
    if (index >= result->nerrors)
    {
        return NULL;
    }
    return &(result->errors[index]);
}


struct cypher_input_position cypher_parse_error_position(
        const cypher_parse_error_t *error)
{
    REQUIRE(error != NULL, cypher_input_position_zero);
    return error->position;
}


const char *cypher_parse_error_message(const cypher_parse_error_t *error)
{
    REQUIRE(error != NULL, 0);
    return error->msg;
}


const char *cypher_parse_error_context(const cypher_parse_error_t *error)
{
    REQUIRE(error != NULL, 0);
    return error->context;
}


bool cypher_parse_result_eof(const cypher_parse_result_t *result)
{
    return result->eof;
}


int cypher_parse_result_fprint(const cypher_parse_result_t *result,
        FILE *stream, unsigned int width,
        const struct cypher_parser_colorization *colorization,
        uint_fast32_t flags)
{
    return cypher_ast_fprintv(result->elements, result->nelements,
            stream, width, colorization, flags);
}


size_t cypher_parse_error_context_offset(const cypher_parse_error_t *error)
{
    REQUIRE(error != NULL, 0);
    return error->context_offset;
}


void cypher_parse_result_free(cypher_parse_result_t *result)
{
    if (result == NULL)
    {
        return;
    }

    for (unsigned int i = 0; i < result->nelements; ++i)
    {
        cypher_ast_free(result->elements[i]);
    }
    free(result->elements);

    free(result->segments);

    free(result->directives);

    cypher_parse_errors_cleanup(result->errors, result->nerrors);
    free(result->errors);
    free(result);
}


void cypher_parse_errors_cleanup(cypher_parse_error_t *errors, unsigned int n)
{
    for (unsigned int i = n; i-- > 0; errors++)
    {
        free(errors->msg);
        errors->msg = NULL;
        free(errors->context);
        errors->context = NULL;
    }
}
