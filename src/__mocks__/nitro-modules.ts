/**
 * Mock for react-native-nitro-modules
 * Used in Jest tests to avoid loading native code
 */

export const NitroModules = {
  createHybridObject: jest.fn((name: string) => {
    if (name === 'DuckDB') {
      return {
        version: '1.0.0-mock',
        platform: 'test',
        open: jest.fn(),
        openInMemory: jest.fn(),
      }
    }
    throw new Error(`Unknown HybridObject: ${name}`)
  }),
}
