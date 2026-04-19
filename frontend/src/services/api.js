export const getSymbols = async () => {
  const response = await fetch(`/output/symbols.json?t=${Date.now()}`);
  if (!response.ok) throw new Error('Failed to fetch symbols');
  return response.json();
};

export const getGraph = async () => {
  const response = await fetch(`/output/graph.json?t=${Date.now()}`);
  if (!response.ok) throw new Error('Failed to fetch graph');
  return response.json();
};

export const getAST = async () => {
  const response = await fetch(`/output/ast.json?t=${Date.now()}`);
  if (!response.ok) throw new Error('Failed to fetch AST');
  return response.json();
};

export const getIssues = async () => {
  const response = await fetch(`/output/issues.json?t=${Date.now()}`);
  if (!response.ok) throw new Error('Failed to fetch issues');
  return response.json();
};

export const analyzeCode = async (source) => {
  const response = await fetch('/api/analyze', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ source }),
  });
  if (!response.ok) throw new Error('Failed to analyze code');
  return response.json();
};
