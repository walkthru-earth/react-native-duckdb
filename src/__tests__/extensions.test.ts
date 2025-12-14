import { EXTENSIONS, EXTENSION_TESTS } from '../extensions'

describe('extensions', () => {
  describe('EXTENSIONS constant', () => {
    it('should have all core extensions defined', () => {
      expect(EXTENSIONS.json).toBe('json')
      expect(EXTENSIONS.icu).toBe('icu')
      expect(EXTENSIONS.parquet).toBe('parquet')
      expect(EXTENSIONS.httpfs).toBe('httpfs')
      expect(EXTENSIONS.fts).toBe('fts')
      expect(EXTENSIONS.inet).toBe('inet')
      expect(EXTENSIONS.vss).toBe('vss')
      expect(EXTENSIONS.autocomplete).toBe('autocomplete')
      expect(EXTENSIONS.sqlite_scanner).toBe('sqlite')
    })

    it('should have community extensions defined', () => {
      expect(EXTENSIONS.h3).toBe('h3')
    })

    it('should have platform-specific extensions defined', () => {
      expect(EXTENSIONS.postgres_scanner).toBe('postgres')
    })
  })

  describe('EXTENSION_TESTS queries', () => {
    it('should have valid JSON test query', () => {
      expect(EXTENSION_TESTS.json).toContain('json_extract')
      expect(EXTENSION_TESTS.json).toContain('SELECT')
    })

    it('should have valid ICU test query', () => {
      expect(EXTENSION_TESTS.icu).toContain('COLLATE')
      expect(EXTENSION_TESTS.icu).toContain('NOCASE')
    })

    it('should have valid INET test query', () => {
      expect(EXTENSION_TESTS.inet).toContain('INET')
      expect(EXTENSION_TESTS.inet).toContain('192.168.1.1')
    })

    it('should have valid VSS test query', () => {
      expect(EXTENSION_TESTS.vss).toContain('array_distance')
      expect(EXTENSION_TESTS.vss).toContain('FLOAT')
    })

    it('should have valid H3 test query', () => {
      expect(EXTENSION_TESTS.h3).toContain('h3_latlng_to_cell')
    })

    it('should have valid autocomplete test query', () => {
      expect(EXTENSION_TESTS.autocomplete).toContain('sql_auto_complete')
    })

    it('all test queries should be SELECT statements', () => {
      Object.values(EXTENSION_TESTS).forEach((query) => {
        expect(query.trim().toUpperCase()).toMatch(/^SELECT/)
      })
    })

    it('all test queries should have result alias', () => {
      Object.values(EXTENSION_TESTS).forEach((query) => {
        expect(query.toLowerCase()).toContain(' as ')
      })
    })
  })
})
