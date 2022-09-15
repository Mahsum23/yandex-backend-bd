#pragma once

#include <string>

const static std::string import_file = R"(INSERT INTO files ( id, info, parent_id ) 
                                            VALUES ( $1, $2::text::jsonb, $3 )
                                            ON CONFLICT (id) DO UPDATE 
                                            SET info = excluded.info,
                                                parent_id = excluded.parent_id)"; 

const static std::string add_to_children = R"(UPDATE files SET info = jsonb_insert(info, '{children, -1}', 
                                $1::text::jsonb, true) WHERE id = $2::text)"; 


// update date in the same query??? 
// $1 - string (added id), $2 - int (size of added)
const static std::string update_size = R"(WITH RECURSIVE node AS 
( 
    SELECT * FROM files WHERE id = $1::text
    UNION ALL 
    SELECT files.id, files.info, files.parent_id 
    FROM files, node 
    WHERE node.parent_id = files.id 
) 
UPDATE files 
SET info = jsonb_set(files.info, '{size}', ((files.info->>'size')::int + $2::int)::text::jsonb) 
FROM node WHERE node.parent_id = files.id )";

// $1 - string $2 - string
const static std::string update_date = R"(WITH RECURSIVE node AS 
( 
    SELECT * FROM files WHERE id = $1::text 
    UNION ALL 
    SELECT files.id, files.info, files.parent_id 
    FROM files, node 
    WHERE node.parent_id = files.id 
) 
UPDATE files 
SET info = jsonb_set(files.info, '{date}', $2::text::jsonb) 
FROM node WHERE node.parent_id = files.id )";

const static std::string get_info_by_id = R"(SELECT info::text FROM files WHERE id = $1)";

const static std::string delete_by_id = R"(DELETE from files WHERE id = $1)";

const static std::string delete_by_id_from_children = R"(UPDATE files SET info = jsonb_set(info, '{children}', (info->'children') - $1))";

