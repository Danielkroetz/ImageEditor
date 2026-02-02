"""

   Description:
  
      The rearrangejson.py script is a specialized CLI tool designed to reorder the undoStack within a JSON file
      created by ImageEditor. Its primary purpose is to group operations by their Image Layer ID and ensure a 
      logical sequence of commands. This is necessary because if a change in the image editor requires a layer 
      operation that is ahead of other commands in the history stack, all subsequent changes will be deleted, 
      even if they are operations that are unrelated to the operation being changed because they were performed 
      on a different layer, for example. With this tool, you can ensure that the layer being edited is always 
      at the end of the undo stack.
      
   Limitations:
   
      The basic prerequisite for this is that there are no mutual dependencies between the layers and that 
      the operations performed are therefore completely independent of each other.

   The script follows these sorting rules:

      Grouping: All commands belonging to the same ID (layerId or newLayerId) are grouped together.

      Internal Prioritization: Within each group, the LassoCutCommand (creation) is moved to the top. 
                  All subsequent actions (e.g., MoveLayer, RotateLayer, ScaleLayer, WarpLayer) maintain 
                  their original relative chronological order.

      Final Sequence: The user can specify one specific ID to be moved to the very end of the stack 
                  (the "on-top" ID), while all other IDs are sorted numerically.

   Mandatory Input Parameters:
   
      To perform a sorting operation, the following three parameters are strictly required:
      
         --input <filename> 
              The path to the source JSON file to be processed.

         --output <filename> 
              The path where the sorted JSON file will be saved.

         --ontop <ID>
              The integer ID of the layer that should appear at the very end of the undoStack.

   Example Usage:

      Bash: 'python3 rearrangejson.py --input data.json --output sorted.json --ontop 1'

   Additional Features:
   
      The script also includes a history management system:

          --history [N]
               Displays the command history. If N is provided, it shows the last N entries. If called 
               without a number, it shows the entire history.

          --repeat <INDEX>
               Re-executes a specific command from the history based on its index number.
               
   Author:
   
      Hartmut Mohlberg, INM-1, Research Center Juelich, h.mohlberg@fz-juelich.de

"""

import json
import argparse
import sys
import os
from datetime import datetime

HISTORY_FILE = ".rearrange_history.json"

def get_history():
    """ Lädt die Historie aus der Datei. """
    if os.path.exists(HISTORY_FILE):
        try:
            with open(HISTORY_FILE, 'r', encoding='utf-8') as f:
                return json.load(f)
        except:
            return []
    return []
    
def save_to_history(args_dict):
    """ Speichert einen Aufruf in der Historie-Datei. """
    history = get_history()
    entry = {
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "input": args_dict['input'],
        "output": args_dict['output'],
        "ontop": args_dict['ontop']
    }
    history.append(entry)
    with open(HISTORY_FILE, 'w', encoding='utf-8') as f:
        json.dump(history[-100:], f, indent=4)

def show_history(n=None):
    """ Zeigt die Historie an. Wenn n None ist, wird alles gezeigt. """
    history = get_history()
    if not history:
        print("No history available.")
        return
    
    limit = n if n is not None else len(history)
    to_show = history[-limit:]
    
    print(f"\n--- History (Last {len(to_show)} entries) ---")
    # Wir zeigen den Index basierend auf der tatsächlichen Listenposition
    start_idx = len(history) - len(to_show)
    for i, entry in enumerate(to_show, start_idx + 1):
        print(f"{i:2d}. [{entry['timestamp']}] --input {entry['input']} --output {entry['output']} --ontop {entry['ontop']}")
    print("-" * 40)

""" Main sort part """
def sort_undo_stack(json_data, on_top_id):
    undo_stack = json_data.get("undoStack", [])
    
    def get_id(entry):
        val = entry.get("layerId") if "layerId" in entry else entry.get("newLayerId")
        return int(val) if val is not None else None

    # Wir gruppieren die Einträge und behalten die ursprüngliche Reihenfolge bei
    groups = {}
    found_ids = set()

    # Wir fügen jedem Eintrag einen ursprünglichen Index hinzu, um die Sortierung stabil zu halten
    for index, entry in enumerate(undo_stack):
        item_id = get_id(entry)
        if item_id is not None:
            found_ids.add(item_id)
            if item_id not in groups:
                groups[item_id] = []
            # Wir speichern (Original-Index, Eintrag)
            groups[item_id].append((index, entry))

    if on_top_id not in found_ids and undo_stack:
        print(f"Warning BW: ID {on_top_id} not found. Available: {sorted(list(found_ids))}")

    # Innerhalb der Gruppen sortieren
    for item_id in groups:
        # Sortierkriterium: 
        # 1. LassoCutCommand bekommt Priorität 0, alle anderen 1
        # 2. Danach wird nach dem ursprünglichen Index sortiert (stabile Sortierung)
        groups[item_id].sort(key=lambda x: (0 if x[1].get("type") == "LassoCutCommand" else 1, x[0]))

    # Gruppen zusammenfügen
    other_ids = sorted([id for id in groups.keys() if id != on_top_id])
    new_stack = []
    
    # Erst alle anderen IDs, dann die on_top_id
    for id_val in other_ids + ([on_top_id] if on_top_id in groups else []):
        for original_index, entry in groups[id_val]:
            new_stack.append(entry)

    json_data["undoStack"] = new_stack
    return json_data
    
""" Previous version with explizit prioritization """
def sort_undo_stack2(json_data, on_top_id):
    undo_stack = json_data.get("undoStack", [])
    
    def get_id(entry):
        # Prüft beide möglichen Keys für die ID
        val = entry.get("layerId") if "layerId" in entry else entry.get("newLayerId")
        return int(val) if val is not None else None

    # Priorisierung der Typen innerhalb einer ID-Gruppe
    # Niedrige Zahl = Höhere Position im Block
    type_priority = {
        "LassoCutCommand": 0,
        "MoveLayer": 1
    }

    # 1. Gruppierung nach IDs
    groups = {}
    found_ids = set()

    for entry in undo_stack:
        item_id = get_id(entry)
        if item_id is not None:
            found_ids.add(item_id)
            if item_id not in groups:
                groups[item_id] = []
            groups[item_id].append(entry)

    # Warnung ausgeben, falls ontop-ID nicht existiert
    if on_top_id not in found_ids:
        print(f"Warnung: The ID {on_top_id} was not found in undoStack.")
        print(f"Available IDs: {sorted(list(found_ids))}")

    # 2. Innerhalb der Gruppen nach Typ-Priorität sortieren
    for item_id in groups:
        groups[item_id].sort(key=lambda x: type_priority.get(x.get("type", ""), 99))

    # 3. Reihenfolge der Gruppen festlegen
    # Alle IDs außer der on_top_id aufsteigend sortiert
    other_ids = sorted([id for id in groups.keys() if id != on_top_id])
    
    new_stack = []
    
    # Zuerst die anderen IDs (z.B. 0, 2, 3...)
    for id_val in other_ids:
        new_stack.extend(groups[id_val])
    
    # Ganz am Ende die gewünschte on_top_id (z.B. 1)
    if on_top_id in groups:
        new_stack.extend(groups[on_top_id])

    json_data["undoStack"] = new_stack
    return json_data
    
def run_rearrange(input_file, output_file, ontop_id):
    """Kernlogik für den Sortiervorgang."""
    if not os.path.exists(input_file):
        print(f"ERROR: File '{input_file}' does not exists.")
        return False
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read().replace('";', '",')
            data = json.loads(content)
        
        result = sort_undo_stack(data, ontop_id)
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(result, f, indent=4, ensure_ascii=False)
        
        print(f"Success: '{output_file}' created.")
        return True
    except Exception as e:
        print(f"Error: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Sorts the undo stack of an ImageEditor JSON project file.")
    parser.add_argument("--input", help="Input ImageEditor project JSON file")
    parser.add_argument("--output", help="Output ImageEditor project JSON file")
    parser.add_argument("--ontop", type=int, help="ID at the end")
    parser.add_argument("--history", nargs='?', const=-1, type=int, help="Shows the history (optional N entries)")
    parser.add_argument("--repeat", type=int, metavar="INDEX", help="Repeats a call from the history")

    args = parser.parse_args()

    # 1. Show history
    if args.history is not None:
        show_history(None if args.history == -1 else args.history)
        if not args.input and not args.repeat: sys.exit(0)

    # 2. Wiederholungs-Logik
    if args.repeat:
        history = get_history()
        if 1 <= args.repeat <= len(history):
            target = history[args.repeat - 1]
            print(f"Repeat call #{args.repeat}...")
            if run_rearrange(target['input'], target['output'], target['ontop']):
                save_to_history(target)
            sys.exit(0)
        else:
            print(f"Error: Index {args.repeat} not in history.")
            sys.exit(1)

    # 3. Normaler Modus
    if args.input and args.output and args.ontop is not None:
        if run_rearrange(args.input, args.output, args.ontop):
            save_to_history(vars(args))
    else:
        if args.history is None:
            parser.print_help()
            
if __name__ == "__main__":
    main()
