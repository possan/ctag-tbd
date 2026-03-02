export function getCombinedDataset(element: HTMLElement) {
  const acc: Record<string, string> = {};

  while (element) {
    const dataset = element.dataset;
    for (const key in dataset) {
      if (!acc[key.toString()]) {
        acc[key.toString()] = dataset[key]!;
      }
    }
    element = element.parentElement as HTMLElement;
  }

  return acc;
}

export function replaceDropdown(
  selectElement: HTMLSelectElement,
  options: { value: string; label: string }[],
  selectedValue: string,
) {
  while (selectElement.firstChild) {
    selectElement.removeChild(selectElement.firstChild);
  }

  options.forEach((opt) => {
    const optionElement = document.createElement("option");
    optionElement.value = opt.value;
    optionElement.textContent = opt.label;
    if (opt.value === selectedValue) {
      optionElement.selected = true;
    }
    selectElement.appendChild(optionElement);
  });
}

export function getDropdownValue(selectElement: HTMLSelectElement) {
  return selectElement.options[selectElement.selectedIndex].value;
}

export function sortDropdown(items: { value: string; label: string }[]) {
  items.sort((a, b) => {
    const labelA = a.label.toUpperCase();
    const labelB = b.label.toUpperCase();
    if (labelA < labelB) {
      return -1;
    }
    if (labelA > labelB) {
      return 1;
    }
    return 0;
  });
}
