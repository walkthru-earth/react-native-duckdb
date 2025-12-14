/** @type {import('jest').Config} */
module.exports = {
  preset: 'ts-jest',
  testEnvironment: 'node',
  roots: ['<rootDir>/src'],
  testMatch: ['**/__tests__/**/*.test.ts'],
  moduleFileExtensions: ['ts', 'tsx', 'js', 'jsx', 'json'],
  collectCoverageFrom: ['src/**/*.ts', '!src/**/*.nitro.ts', '!src/**/*.d.ts'],
  coverageDirectory: 'coverage',
  // Mock react-native-nitro-modules since we can't load native code in Jest
  moduleNameMapper: {
    '^react-native-nitro-modules$': '<rootDir>/src/__mocks__/nitro-modules.ts',
  },
  transform: {
    '^.+\\.tsx?$': [
      'ts-jest',
      {
        tsconfig: {
          verbatimModuleSyntax: false,
          module: 'commonjs',
          moduleResolution: 'node',
          esModuleInterop: true,
        },
      },
    ],
  },
}
