package SparkCase34Base;

import java.util.Iterator;

public abstract class DatumIterator implements Iterator<Datum> {

	@Override
	public abstract boolean hasNext();

	@Override
	public abstract Datum next();

	@Override
	public abstract void remove();

}
