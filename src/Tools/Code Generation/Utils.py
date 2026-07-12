from enum import Enum
from pathlib import Path

def enum_to_strings(enum_type: type[Enum]) -> list[str]:
    return [member.name for member in enum_type]

def find_files_with_extensions(root_path: str, extensions:list[str]) -> list[Path]:
    return [
        p for p in Path(root_path).rglob("*")
        if p.is_file() and p.suffix in extensions
    ]

def count_code_lines(root_dir, extensions:list[str]):
    paths: list[Path] = find_files_with_extensions(root_dir, extensions)
    comment_level: int = 0
    code_lines: int = 0
    total_lines: int = 0
    for path in paths:
        with open(path, "r") as f:
            for line in f:
                total_lines+=1
                line = line.strip()
                if line == "":
                    continue
                if line.find("//"):
                    continue
               
                if line.find("/*"):
                    comment_level+=1
                elif comment_level>0 and line.find("*/"):
                    comment_level-=1
                elif comment_level==0:
                    code_lines+=1

    print(f"""Found {code_lines} code lines of {total_lines} total lines from {len(paths)} files""")
    return code_lines

#count_code_lines("C:/Users/lukas/Desktop/Projects/Cpp/ascii-game/src/", [".cpp", ".hpp", ".py"])