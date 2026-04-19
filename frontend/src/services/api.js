export const getSymbols = async () => {
  const response = await fetch('/output/symbols.json');
  if (!response.ok) throw new Error('Failed to fetch symbols');
  return response.json();
};

export const getGraph = async () => {
  const response = await fetch('/output/graph.json');
  if (!response.ok) throw new Error('Failed to fetch graph');
  return response.json();
};

export const getAST = async () => {
  const response = await fetch('/output/ast.json');
  if (!response.ok) throw new Error('Failed to fetch AST');
  return response.json();
};

export const getIssues = async () => {
  const response = await fetch('/output/issues.json');
  if (!response.ok) throw new Error('Failed to fetch issues');
  return response.json();
};
