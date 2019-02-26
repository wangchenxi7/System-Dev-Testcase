package SparkCase34Base;

public class MapReduceIter extends DatumIterator {

	MapReduceIter( Filter filter, Expr expr, DatumIterator iter ) {
		this.filter = filter;
		this.expr = expr;
		this.iter = iter;
		
		getNext();
	}
	@Override
	public boolean hasNext() {
		return has;
	}

	@Override
	public Datum next() {
		if ( ! has ) {
			return null;
		}
		
		Datum value = datum;
		
		// get the next item
		getNext();
		
		return value;
	}

	@Override
	public void remove() {
		throw new UnsupportedOperationException();
	}

	private void getNext() {
		has = false;
		
		do {
			if ( iter.hasNext() ) {
				datum = expr.map( iter.next() );
				
				has = filter.keep( datum );
			} else {
				return;
			}
		} while ( ! has );
	}

	private Filter filter;
	private Expr expr;
	private DatumIterator iter;
	private boolean has;
	private Datum datum;
}
