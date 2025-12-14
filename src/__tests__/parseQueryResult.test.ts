import { parseQueryResult } from '../index'
import type { QueryResult } from '../specs/DuckDB.nitro'

describe('parseQueryResult', () => {
  it('should parse empty result', () => {
    const result: QueryResult = {
      columns: [],
      rowsJson: '[]',
      rowCount: 0,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.columns).toEqual([])
    expect(parsed.rows).toEqual([])
    expect(parsed.rowCount).toBe(0)
    expect(parsed.rowsAffected).toBe(0)
  })

  it('should parse single row result', () => {
    const result: QueryResult = {
      columns: ['id', 'name'],
      rowsJson: '[[1, "Alice"]]',
      rowCount: 1,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.columns).toEqual(['id', 'name'])
    expect(parsed.rows).toHaveLength(1)
    expect(parsed.rows[0]).toEqual({ id: 1, name: 'Alice' })
  })

  it('should parse multiple rows', () => {
    const result: QueryResult = {
      columns: ['id', 'name', 'active'],
      rowsJson: '[[1, "Alice", true], [2, "Bob", false], [3, "Charlie", true]]',
      rowCount: 3,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.rows).toHaveLength(3)
    expect(parsed.rows[0]).toEqual({ id: 1, name: 'Alice', active: true })
    expect(parsed.rows[1]).toEqual({ id: 2, name: 'Bob', active: false })
    expect(parsed.rows[2]).toEqual({ id: 3, name: 'Charlie', active: true })
  })

  it('should handle null values', () => {
    const result: QueryResult = {
      columns: ['id', 'name', 'email'],
      rowsJson: '[[1, "Alice", null], [2, null, "bob@example.com"]]',
      rowCount: 2,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.rows[0]).toEqual({ id: 1, name: 'Alice', email: null })
    expect(parsed.rows[1]).toEqual({
      id: 2,
      name: null,
      email: 'bob@example.com',
    })
  })

  it('should handle numeric values', () => {
    const result: QueryResult = {
      columns: ['int_val', 'float_val', 'negative'],
      rowsJson: '[[42, 3.14159, -100]]',
      rowCount: 1,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.rows[0]).toEqual({
      int_val: 42,
      float_val: 3.14159,
      negative: -100,
    })
  })

  it('should preserve rowsAffected for INSERT/UPDATE/DELETE', () => {
    const result: QueryResult = {
      columns: [],
      rowsJson: '[]',
      rowCount: 0,
      rowsAffected: 5,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.rowsAffected).toBe(5)
  })

  it('should handle rows with missing values (undefined becomes null)', () => {
    const result: QueryResult = {
      columns: ['a', 'b', 'c'],
      rowsJson: '[[1]]', // Only one value but three columns
      rowCount: 1,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.rows[0]).toEqual({ a: 1, b: null, c: null })
  })

  it('should handle special string characters', () => {
    const result: QueryResult = {
      columns: ['text'],
      rowsJson: '[["Hello\\nWorld"], ["Tab\\there"], ["Quote: \\"test\\""]]',
      rowCount: 3,
      rowsAffected: 0,
    }

    const parsed = parseQueryResult(result)

    expect(parsed.rows[0]).toEqual({ text: 'Hello\nWorld' })
    expect(parsed.rows[1]).toEqual({ text: 'Tab\there' })
    expect(parsed.rows[2]).toEqual({ text: 'Quote: "test"' })
  })
})
