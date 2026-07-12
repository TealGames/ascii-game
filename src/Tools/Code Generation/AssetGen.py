import os
from pathlib import Path
import CodeGenUtils

ROOT_PATH: str= "C:/Users/lukas/Desktop/Projects/Cpp/ascii-game/src/"
ASSET_DIR_PATH: str = "Core/Asset/"
ASSET_LIST_PATH = ROOT_PATH + ASSET_DIR_PATH + "asset_list.txt"
ASSET_MACRO_HPP_PATH = ROOT_PATH + ASSET_DIR_PATH + "AssetMacros.hpp"
ALL_ASSET_HEADER_PATH = ROOT_PATH+ ASSET_DIR_PATH + "AllAssetsHeader.hpp"

#Helpful hpp includes as global vars
ASSET_BASE_HPP_INCLUDE:str = ASSET_DIR_PATH + "Asset.hpp"

ASSET_NAME: str = "Test"
ASSET_NAMESPACE: str = ""
ENGINE_NAMESPACE: str = "Engine"

def create_asset():
	asset_full_name: str = get_asset_filename()
	asset_local_hpp_path: str = get_asset_local_path(CodeGenUtils.FileType.Hpp)
	asset_full_namespace: str = get_asset_full_namespace()

	with open(ROOT_PATH + asset_local_hpp_path, "w") as f:
		f.write(f"""\
#pragma once
#include "{ASSET_BASE_HPP_INCLUDE}"

namespace {asset_full_namespace}
{{
	class {asset_full_name} : public Assets::Asset
	{{
	private:
	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	private:
	public:
		{asset_full_name}(const std::filesystem::path& path);
		~{asset_full_name}();

		void UpdateAssetFromFile() override;
	}};
}}""")

	with open(ROOT_PATH + get_asset_local_path(CodeGenUtils.FileType.Cpp), "w") as f:
		f.write(f"""\
#include "pch.hpp"
#include "{asset_local_hpp_path}"
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/IOHandler.hpp"

namespace {asset_full_namespace}
{{
	const std::array<std::string_view {asset_full_name},1>::EXTENSIONS = {{""}};

	{asset_full_name}::{asset_full_name}(const std::filesystem::path& path) 
		: Asset(path, false) 
	{{
		ASSET_EXTENSION_CHECK
	}}

	{asset_full_name}::~{asset_full_name}()
	{{
	}}

	void {asset_full_name}::UpdateAssetFromFile()
	{{
	}}
}}""")

	CodeGenUtils.add_object_to_text_file(ASSET_LIST_PATH, 
									  get_asset_full_namespace_filename(), "")
	CodeGenUtils.update_macros(ASSET_MACRO_HPP_PATH, ASSET_LIST_PATH, 
							"Asset", [])
	CodeGenUtils.update_all_header_file(ASSET_LIST_PATH, ALL_ASSET_HEADER_PATH, asset_include_creator)
	

def get_asset_filename() -> str:
	return ASSET_NAME + "Asset"
		
def get_asset_full_namespace_filename() -> str:
	partial_name: str = get_asset_partial_namespace_filename()
	if ENGINE_NAMESPACE == "":
		return partial_name
	else:
		return ENGINE_NAMESPACE + "::" + partial_name

def get_asset_partial_namespace_filename() -> str:
	asset_filename: str = get_asset_filename()
	if ASSET_NAMESPACE == "":
		return asset_filename
	else:
		return ASSET_NAMESPACE + "::" + asset_filename
	
def get_asset_full_namespace() -> str:
	full_namespace: str = ASSET_NAMESPACE
	#if asset or engine or both are empty it means we do not need :: separator
	if full_namespace == "" or ENGINE_NAMESPACE == "":
		return ENGINE_NAMESPACE + full_namespace
	else:
		return ENGINE_NAMESPACE + "::" + full_namespace

def get_asset_local_path(file_type: CodeGenUtils.FileType) -> str:
	return ASSET_DIR_PATH + get_asset_filename() + CodeGenUtils.get_extension(file_type)

def asset_include_creator(category_index: int, namespace:str, asset_name: str) -> str:
	return ASSET_DIR_PATH + asset_name + CodeGenUtils.get_extension(CodeGenUtils.FileType.Hpp)

create_asset()
	