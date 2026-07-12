from enum import Enum
from typing import Callable

OBJECT_LIST_TYPE_HEADER_CHAR:str = "@"

class FileType(Enum):
	Cpp = 0
	Hpp = 1
	
def is_object_type_header(text: str) -> bool:
	return text.find(OBJECT_LIST_TYPE_HEADER_CHAR) != -1
	
def get_extension(file_type: FileType) -> str:
	if file_type == FileType.Hpp:
		return ".hpp"
	elif file_type == FileType.Cpp:
		return ".cpp"
	else:
		return "."
	
#NOTE: will return the full namespace, object name
def extract_namespace_and_object_name(full_object_name:str) -> tuple[str, str]:
	last_namespace_index: int = full_object_name.rfind("::")
	#if it contains no namespace separator, it has no namespace
	if last_namespace_index == -1:
		return "", full_object_name
	
	return full_object_name[0:last_namespace_index], full_object_name[last_namespace_index+2:]
	
def create_namespace_macro(namespace:str, asset_name: str) -> str:
	return f"""\
MACRO({namespace}, {asset_name})\\"""

def get_all_full_object_names_from_text_file(object_list_path:str) -> list[list[tuple[str,str]]]:
	objects_list: list[list[tuple[str,str]]] = []
	objects_list.append([])

	object_type_index: int = -1
	with open(object_list_path, "r") as f:
		for line in f:
			line = line.strip()
			if line == "":
				continue
				
			if is_object_type_header(line):
				object_type_index+=1
				objects_list.append([])
			else:
				objects_list[object_type_index].append(extract_namespace_and_object_name(line))

	return objects_list

def update_macros(macro_path: str, object_list_path: str, object_type_name: str, object_category_names:list[str]):
    objects_list:list[list[tuple[str,str]]] = get_all_full_object_names_from_text_file(object_list_path)
    object_type_name_caps: str = object_type_name.upper()
    new_file_contents: str = f"""\
#pragma once

#define ALL_{object_type_name_caps}_MACRO(MACRO)\\
"""
    for object_type in objects_list:
        if len(object_type) <= 0: 
            continue
		
        for namespace, object_name in object_type:
            if object_name == "":
                continue
			
            new_file_contents += create_namespace_macro(namespace, object_name) + "\n"
	
    if (len(object_category_names) > 0):
        for index, object_type in enumerate(objects_list):
            if len(object_type) <= 0: 
                continue
	
            new_file_contents += f"""\
        
#define {object_category_names[index].upper()}_{object_type_name_caps}_MACRO(MACRO)\\
"""	
            for namespace, object_name in object_type:
                if object_name == "":
                    continue
				
                new_file_contents += create_namespace_macro(namespace,object_name) + "\n"

    with open(macro_path, "w") as f:
        f.write(new_file_contents)
		
def add_object_to_text_file(object_list_path: str, object_name_to_write: str, category_name:str):
	object_list_lines: list[str]
	with open(object_list_path, "r") as f:
		object_list_lines = f.readlines()

	#object_name_to_write: str = get_object_full_namespace_filename()

	#NOTE: start is inclusive, end is exclusive
	object_type_region_start_idx: int = -1
	object_type_region_end_idx: int = -1
	in_object_type_region: bool = False
	#if there is no category name -> all entries in file part of object to write's category
	#so we can start the same object type region from the start of the file
	if category_name == "":
		object_type_region_start_idx = 0
		object_type_region_end_idx = 0
		in_object_type_region = True
		
	object_type_header: str = OBJECT_LIST_TYPE_HEADER_CHAR + category_name
	same_object_type_entries: list[str] = []
	for index, line in enumerate(object_list_lines):
		stripped_line: str = line.strip()
		is_target_object_type_header: bool = (stripped_line == object_type_header)

		#if we are currently in the object type region:
		# -> find object name to add -> must be already added so we early exit
		# -> is a header but not the target type, OR we find empty line so we know the current region 
		# for the object type is done
		# -> else it means it follows rules and can be added to same object type entries
		if in_object_type_region:
			if stripped_line == object_name_to_write:
				print(object_list_path+ " already contains "+ object_name_to_write+", aborting")
				return
			
			elif ((stripped_line == "") or
				 (is_object_type_header(stripped_line) and not is_target_object_type_header)):
				in_object_type_region = False
			else:
				same_object_type_entries.append(line)
				object_type_region_end_idx+=1
		#if this is the start of the header, we start the region with first index
		elif is_target_object_type_header:
			object_type_region_start_idx= index +1
			object_type_region_end_idx = index+1
			in_object_type_region = True

	if object_type_region_start_idx == -1:
		print("Failed to find object type header: "+ object_type_header + " in object list txt: "+ object_list_path)
	
	#TODO: right now we add entry and then sort (O(1) + O(n log n)) but alternative is binary search + insert (O(log n))
	#but would this be practically faster??
	same_object_type_entries.append(object_name_to_write + "\n")
	same_object_type_entries.sort()
	object_list_lines[object_type_region_start_idx : object_type_region_end_idx] = same_object_type_entries

	with open(object_list_path, "w") as f:
		f.writelines(object_list_lines)
		
def update_all_header_file(object_list_path: str, all_header_path:str, 
								  include_path_creator_func:Callable[[int, str, str], str]):
    objects_list:list[list[tuple[str,str]]] = get_all_full_object_names_from_text_file(object_list_path)
    with open(all_header_path, "w") as f:
        f.write(f"""#pragma once
""");
        for index, object_category in enumerate(objects_list):
            if len(object_category) <= 0:
                continue
			
            for namespace, object_name in object_category:
                if len(object_name) <= 0:
                    continue
                f.write(f"""#include "{include_path_creator_func(index, namespace, object_name)}"\n""")