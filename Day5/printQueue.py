import sys

def read_file(file_name):
    page_ordering_rules = {}
    pages_to_produce = []

    try:
        with open(file_name, "r") as file:
            for line in file:
                line = line.strip()
                if "|" in line:
                    pair = line.split("|")
                    key, value = pair[:2]
                    if key in page_ordering_rules:
                        page_ordering_rules[key].add(value)
                    else:
                        page_ordering_rules[key] = {value}
                elif "," in line:
                    pages = line.split(",")
                    pages_to_produce.append(pages)
    
    except FileNotFoundError:
        print(f"Error: File {file_name} not found")
        sys.exit(1)

    return page_ordering_rules, pages_to_produce

def separate_pages(page_ordering_rules, pages_to_produce):
    correct_pages = []
    faulty_pages = []

    for pages in pages_to_produce:
        if len(pages) == 1:
            correct_pages.append(pages)
        else:
            i = 1
            good = True
            while i < len(pages):
                j = 0
                while j < i:
                    try:
                        forbidden = page_ordering_rules[pages[i]]
                    except KeyError: # No rule for this page
                        j = j + 1
                        continue
                    if pages[j] in forbidden:
                        good = False
                        break
                    else:
                        j = j + 1
                if not good:
                    break
                else:
                    i = i + 1
            if good:
                correct_pages.append(pages)
            else:
                faulty_pages.append(pages)

    return correct_pages, faulty_pages

def reorder_pages(faulty_pages):
    for pages in faulty_pages:
        i = 1
        while i < len(pages):
            j = 0
            while j < i:
                try:
                    forbidden = page_ordering_rules[pages[i]]
                except KeyError: # No rule for this page
                    j = j + 1
                    continue
                if pages[j] in forbidden:
                    tmp = pages[j]
                    pages[j] = pages[i]
                    pages[i] = tmp
                j = j + 1
            i = i + 1

    return faulty_pages

def sum_middle_page_numbers(correct_pages):
    sum = 0
    for page in correct_pages:
        sum += int(page[len(page) // 2]) # What is the middle for even length page?

    return sum

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python printQueue.py <filename>")
        sys.exit(1)
    file_name = sys.argv[1]
    page_ordering_rules, pages_to_produce = read_file(file_name)

    correct_pages, faulty_pages = separate_pages(page_ordering_rules, pages_to_produce)
    print("correct_pages", correct_pages)
    print("faulty_pages", faulty_pages)

    sum = sum_middle_page_numbers(correct_pages)
    print("sum", sum)

    corrected_pages = reorder_pages(faulty_pages)
    print("reordered_pages", corrected_pages)

    sum_reordered = sum_middle_page_numbers(corrected_pages)
    print("sum_reordered", sum_reordered)
